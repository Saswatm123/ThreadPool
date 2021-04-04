#ifndef TASK_THREADPOOL_HPP
#define TASK_THREADPOOL_HPP

#include <future>
#include <tuple>

struct GenericBoundFunction
{
    virtual void execute() = 0;
    virtual ~GenericBoundFunction(){};
};

// General BoundFunction template for non-void functions. Void functions have specialized BoundFunction below.
template<typename ReturnType, typename... ArgPack>
struct BoundFunction : GenericBoundFunction
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
        this->prom.set_value( this->execute_impl(std::index_sequence_for<ArgPack...>() ) );
    }

    std::future<ReturnType> get_future()
    {
        return this->prom.get_future();
    }

    template<typename... ArgumentFwdTypes>
    BoundFunction(ReturnType(*const func)(ArgPack...), ArgumentFwdTypes... args)
    :func(func), args( std::forward<ArgumentFwdTypes>(args)... )
    {}
};

// Specialization for functions with void return type.
template<typename... ArgPack>
struct BoundFunction<void, ArgPack...> : GenericBoundFunction
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

    template<typename... ArgumentFwdTypes>
    BoundFunction(void(*const func)(ArgPack...), ArgumentFwdTypes&&... args)
    :func(func), args( std::forward<ArgumentFwdTypes>(args)...)
    {}
};

template<typename ReturnType, typename... ArgPack, typename... ArgumentFwdTypes>
BoundFunction<ReturnType, ArgPack...> make_bound_function(ReturnType(*const func)(ArgPack...), ArgumentFwdTypes&&... args)
{
    return BoundFunction<ReturnType, ArgPack...>(func, std::forward<ArgumentFwdTypes>(args)...);
}

#endif // TASK_THREADPOOL_HPP
