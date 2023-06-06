#include "func_tools.hpp"

template<typename ReturnType, typename... ParamTypes, typename... ArgumentFwdTypes>
std::future<ReturnType> ThreadPool::submit_task(ReturnType(function)(ParamTypes...), ArgumentFwdTypes&&... args)
{
    /*
        Current function ptr type includes the type of all parameters in the function (ex. void(int, char) ).
        However, if function has default arguments (ex. void(int, char='a') ), the types passed into this function
        as arguments, and the types in the parameter list may be different (since default args don't have to be provided).
        We can't call a function through its pointer without providing all of its arguments, including its default arguments,
        so to get around this, we use reinterpret_cast to cast the function pointer we get to a function pointer of a truncated
        parameter list. We truncate the parameter list to the length of the argument list. This way, original types are preserved,
        as opposed to replacing the param list with the arg list, and we get to call our function without default arguments.
        This is the point of the "shortened_function" below.
    */
    auto shortened_function = shorten_function_paramlist(function, std::forward<ArgumentFwdTypes>(args)... );
    typedef decltype(shortened_function) FunctionType;

    BoundFunction<ReturnType, FunctionType, ArgumentFwdTypes...>* new_task = new BoundFunction<ReturnType, FunctionType, ArgumentFwdTypes...>(shortened_function, std::forward<ArgumentFwdTypes>(args)... );

    std::unique_lock<std::mutex> u_lock(this->queue_guard);

    this->task_storage.emplace_front( new_task );
    auto task_iterator = this->task_storage.begin();

    this->task_queue.emplace( task_iterator );

    u_lock.unlock();

    this->check_task.notify_one();

    return std::move( new_task->get_future() );
}
