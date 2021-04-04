#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "bound_function.hpp"
#include <condition_variable>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <algorithm>
#include <list>

class threadpool
{
    const unsigned thread_count;
    const bool complete_upon_destruction;

    std::condition_variable check_task;
    std::mutex queue_guard;
    std::atomic<bool> end_tasks;
    std::vector<std::thread> thread_container;

    // Tasks are stored in a Linked List for fast removal & addition
    std::list< GenericBoundFunction* > task_storage;

    // Iterators pointing to the tasks are stored in a task queue
    std::queue< std::list<GenericBoundFunction*>::iterator > task_queue;

    void wait_for_task()
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

public:

    unsigned get_thread_count()
    {
        return thread_count;
    }

    template<typename ReturnType, typename... ArgTypes, typename... ArgumentFwdTypes>
    std::future<ReturnType> submit_task(ReturnType(function)(ArgTypes...), ArgumentFwdTypes&&... args)
    {
        BoundFunction<ReturnType, ArgTypes...>* new_task = new BoundFunction<ReturnType, ArgTypes...>(function, std::forward<ArgumentFwdTypes>(args)... );

        std::unique_lock<std::mutex> u_lock(this->queue_guard);

        this->task_storage.emplace_front( new_task );
        auto task_iterator = this->task_storage.begin();

        this->task_queue.emplace( task_iterator );

        u_lock.unlock();

        this->check_task.notify_one();

        return std::move( new_task->get_future() );
    }

    threadpool(unsigned thread_count, bool complete_upon_destruction = 1)
    :end_tasks(false), thread_count(thread_count), complete_upon_destruction(complete_upon_destruction)
    {
        for(int i = 0; i < thread_count; i++)
        {
            this->thread_container.push_back(std::thread(this->wait_for_task, this) );
        }
    }

    ~threadpool()
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
};

#endif // THREADPOOL_HPP
