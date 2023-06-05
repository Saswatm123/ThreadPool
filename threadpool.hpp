#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "impl/bound_function.hpp"
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

    // Threads are initialized on this function. Threads sleep inside this function
    // on condition_variable check_task and wait till there is an open task to complete,
    // then carry out the task & delete it from the task registry.
    void wait_for_task();

public:

    unsigned get_thread_count();

    template<typename ReturnType, typename... ParamTypes, typename... ArgumentFwdTypes>
    std::future<ReturnType> submit_task(ReturnType(function)(ParamTypes...), ArgumentFwdTypes&&... args);

    threadpool(unsigned thread_count, bool complete_upon_destruction = 1);

    ~threadpool();
};

#include "impl/threadpool.tpp"

#endif // THREADPOOL_HPP
