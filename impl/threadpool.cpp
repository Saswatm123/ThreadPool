#include "../threadpool.hpp"

void threadpool::wait_for_task()
{
    std::unique_lock<std::mutex> u_lock(this->queue_guard);
    while( !(this->task_queue.empty() && this->end_tasks) )
    {
        this->check_task.wait(u_lock, [&](){return !this->task_queue.empty() || this->end_tasks;} );

        if(this->end_tasks && ( (this->complete_upon_destruction && this->task_queue.empty() ) || !this->complete_upon_destruction ) )
        {
            break;
        }

        std::list<GenericBoundFunction*>::iterator current_task_ptr = this->task_queue.front();
        this->task_queue.pop();

        u_lock.unlock();

        (*current_task_ptr)->execute();

        // Delete task from storage
        u_lock.lock();

        if( !this->task_queue.empty() )
        {
            delete *current_task_ptr;
        }
        this->task_storage.erase(current_task_ptr);
    }
}

unsigned threadpool::get_thread_count()
{
    return thread_count;
}

threadpool::threadpool(unsigned thread_count, bool complete_upon_destruction)
:end_tasks(false), thread_count(thread_count), complete_upon_destruction(complete_upon_destruction)
{
    for(int i = 0; i < thread_count; i++)
    {
        this->thread_container.push_back(std::thread(this->wait_for_task, this) );
    }
}

threadpool::~threadpool()
{
    this->end_tasks = true;
    std::for_each(this->thread_container.begin(), this->thread_container.end(),
        [&](std::thread& t){
            this->check_task.notify_all();
            if(this->complete_upon_destruction)
            {
                t.join();
            }
            else
            {
                t.detach();
            }
        }
    );
}
