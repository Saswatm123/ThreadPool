#ifndef PRIORITYTHREADPOOL_HPP
#define PRIORITYTHREADPOOL_HPP

#include "impl/bound_function.hpp"
#include <condition_variable>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <algorithm>
#include <list>

class PriorityThreadPool
{
    const unsigned thread_count;
    const bool complete_upon_destruction;

    std::condition_variable check_task;
    std::mutex queue_guard;
    std::atomic<bool> end_tasks;
    std::vector<std::thread> thread_container;

    // Tasks are stored in a Linked List for fast removal & addition
    std::list< std::shared_ptr<GenericBoundFunction> > task_storage;

    typedef std::pair<
        std::list< std::shared_ptr<GenericBoundFunction> >::iterator,
        int
    > WeightedTask;

    struct WeightedTaskComparator
    {
        bool operator()(const WeightedTask& lhs, const WeightedTask& rhs)
        {
            return lhs.second < rhs.second;
        }
    };

    std::priority_queue<
        WeightedTask,
        std::vector<WeightedTask>,
        WeightedTaskComparator
    > task_queue;

    // Iterators pointing to the tasks are stored in a task queue
    // std::queue< std::list< std::shared_ptr<GenericBoundFunction> >::iterator > task_queue;

    // Threads are initialized on this function. Threads sleep inside this function
    // on condition_variable check_task and wait till there is an open task to complete,
    // then carry out the task & delete it from the task registry.
    void wait_for_task();

public:

    unsigned get_thread_count();

    template<typename ReturnType, typename... ArgTypes, typename... ArgumentFwdTypes>
    std::future<ReturnType> submit_task(int priority, ReturnType(function)(ArgTypes...), ArgumentFwdTypes&&... args);

    PriorityThreadPool(unsigned thread_count, bool complete_upon_destruction = 1);

    ~PriorityThreadPool();
};

#include "impl/prioritythreadpool.tpp"

#endif // PRIORITYTHREADPOOL_HPP
