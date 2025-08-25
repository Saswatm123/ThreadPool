#include <memory>
template<typename ReturnType, typename... ArgTypes, typename... ArgumentFwdTypes>
std::future<ReturnType> PriorityThreadPool::submit_task(int priority, ReturnType(function)(ArgTypes...), ArgumentFwdTypes&&... args)
{
    std::shared_ptr< BoundFunction<ReturnType, ArgTypes...> > new_task = std::make_shared< BoundFunction<ReturnType, ArgTypes...> >( function, std::forward<ArgumentFwdTypes>(args)... );

    std::unique_lock<std::mutex> u_lock(this->queue_guard);

    this->task_storage.emplace_front( new_task );
    auto task_iterator = this->task_storage.begin();

    this->task_queue.emplace( task_iterator, priority );

    u_lock.unlock();

    this->check_task.notify_one();

    return std::move( new_task->get_future() );
}
