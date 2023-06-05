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
template<typename ReturnType, typename FunctionType, typename... ArgPack>
struct BoundFunction : GenericBoundFunction
{
private:
    template<std::size_t... indices>
    ReturnType execute_impl(std::index_sequence<indices...>);

    std::tuple<ArgPack...> args;
    FunctionType func;
    std::promise<ReturnType> prom;

public:

    // Executes function, stores results in promise "prom".
    inline void execute();

    // Returns future that will hold output result once execute() is called.
    std::future<ReturnType> get_future();

    // func is a function to execute, and args is a variadic argument that contains
    // the arguments to pass into the function. F(a, b) would be called as
    // BoundFunction(F, a, b).
    template<typename... ArgumentFwdTypes>
    BoundFunction(FunctionType func, ArgumentFwdTypes&&... args);
};

// Specialization for functions with void return type.
template<typename FunctionType, typename... ArgPack>
struct BoundFunction<void, FunctionType, ArgPack...> : GenericBoundFunction
{
private:
    template<std::size_t... indices>
    void execute_impl(std::index_sequence<indices...>);

    std::tuple<ArgPack...> args;
    FunctionType func;
    std::promise<void> prom;

public:
    // Executes function.
    inline void execute();

    // Returns future corresponding to promise "prom". Future is never updated since this
    // specialization is for void return types only.
    std::future<void> get_future();

    // func is a function to execute, and args is a variadic argument that contains
    // the arguments to pass into the function. F(a, b) would be called as
    // BoundFunction(F, a, b).
    template<typename... ArgumentFwdTypes>
    BoundFunction(FunctionType func, ArgumentFwdTypes&&... args);
};

template<typename ReturnType, typename... ParamTypes, typename... ArgumentFwdTypes>
auto make_bound_function(ReturnType(func)(ParamTypes...), ArgumentFwdTypes&&... args);

#include "bound_function.tpp"

#endif // TASK_THREADPOOL_HPP
