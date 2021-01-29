#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "task.hpp"
#include <condition_variable>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <algorithm>

class threadpool
{
    std::condition_variable check_task;
    std::mutex queue_guard;
    std::atomic<bool> end_tasks;
    std::vector<std::thread> thread_container;

    std::queue< PolymorphicBoundFunction* > tasks;

    void wait_for_task()
    {
        while( !(this->tasks.empty() && this->end_tasks) )
        {
            std::unique_lock<std::mutex> u_lock(this->queue_guard);
            this->check_task.wait(u_lock, [&](){return !this->tasks.empty() || this->end_tasks;} );

            if(this->end_tasks && this->tasks.empty() )
            {
                break;
            }

            auto current_task = std::move(this->tasks.front() );
            this->tasks.pop();
            u_lock.unlock();

            current_task->execute();
            delete current_task;
        }
    }

public:

    template<typename ReturnType, typename... ArgTypes>
    std::future<ReturnType> submit_task(ReturnType(function)(ArgTypes...), ArgTypes... args)
    {
        std::lock_guard<std::mutex> u_lock(this->queue_guard);
        BoundFunction<ReturnType, ArgTypes...>* new_task = new BoundFunction<ReturnType, ArgTypes...>(function, args...);
        this->tasks.emplace( new_task );
        this->check_task.notify_one();
        return std::move( new_task->get_future() );
    }

    threadpool(unsigned thread_count)
    :end_tasks(false)
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
                t.join();
            }
        );
    }
};

#endif // THREADPOOL_HPP
