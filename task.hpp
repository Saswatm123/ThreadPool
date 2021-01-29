#ifndef TASK_THREADPOOL_HPP
#define TASK_THREADPOOL_HPP

#include <future>
#include <tuple>

struct PolymorphicBoundFunction
{
    virtual void execute() = 0;
    virtual ~PolymorphicBoundFunction(){}
};

// General template for non-void functions. Void functions have specialized BoundFunction below.
template<typename ReturnType, typename... ArgPack>
struct BoundFunction : PolymorphicBoundFunction
{
private:
    template<std::size_t... indices>
    ReturnType execute_impl(std::index_sequence<indices...>)
    {
        return this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
    }

    std::tuple<ArgPack...> args;
    ReturnType(*const func)(ArgPack...);
    std::promise<ReturnType> prom;

public:

    inline void execute()
    {
        ReturnType res = this->execute_impl(std::index_sequence_for<ArgPack...>() );
        this->prom.set_value(res);
    }

    std::future<ReturnType> get_future()
    {
        return this->prom.get_future();
    }

    BoundFunction(ReturnType(*const func)(ArgPack...), ArgPack... args)
    :func(func), args(args...)
    {}
};

// Specialization for functions with void return type.
template<typename... ArgPack>
struct BoundFunction<void, ArgPack...> : PolymorphicBoundFunction
{
private:
    template<std::size_t... indices>
    void execute_impl(std::index_sequence<indices...>)
    {
        this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
    }

    std::tuple<ArgPack...> args;
    void(*const func)(ArgPack...);
    std::promise<void> prom;

public:

    inline void execute()
    {
        this->execute_impl(std::index_sequence_for<ArgPack...>() );
    }

    std::future<void> get_future()
    {
        return this->prom.get_future();
    }

    BoundFunction(void(*const func)(ArgPack...), ArgPack... args)
    :func(func), args(args...)
    {}
};

template<typename ReturnType, typename... ArgPack>
BoundFunction<ReturnType, ArgPack...> make_task(ReturnType(*const func)(ArgPack...), ArgPack... args)
{
    return BoundFunction<ReturnType, ArgPack...>(func, args...);
}

#endif // TASK_THREADPOOL_HPP
