#include "func_tools.hpp"

template<typename ReturnType, typename FunctionType, typename... ArgPack>
inline void BoundFunction<ReturnType, FunctionType, ArgPack...>::execute()
{
    this->prom.set_value( this->execute_impl(std::index_sequence_for<ArgPack...>() ) );
}

template<typename ReturnType, typename FunctionType, typename... ArgPack>
template<std::size_t... indices>
ReturnType BoundFunction<ReturnType, FunctionType, ArgPack...>::execute_impl(std::index_sequence<indices...>)
{
    return this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
}

template<typename ReturnType, typename FunctionType, typename... ArgPack>
std::future<ReturnType> BoundFunction<ReturnType, FunctionType, ArgPack...>::get_future()
{
    return this->prom.get_future();
}

template<typename ReturnType, typename FunctionType, typename... ArgPack>
template<typename... ArgumentFwdTypes>
BoundFunction<ReturnType, FunctionType, ArgPack...>::BoundFunction(FunctionType func, ArgumentFwdTypes&&... args)
:func(func), args( std::forward<ArgumentFwdTypes>(args)... )
{}

template<typename FunctionType, typename... ArgPack>
template<std::size_t... indices>
void BoundFunction<void, FunctionType, ArgPack...>::execute_impl(std::index_sequence<indices...>)
{
    this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
}

template<typename FunctionType, typename... ArgPack>
inline void BoundFunction<void, FunctionType, ArgPack...>::execute()
{
    this->execute_impl(std::index_sequence_for<ArgPack...>() );
}

template<typename FunctionType, typename... ArgPack>
std::future<void> BoundFunction<void, FunctionType, ArgPack...>::get_future()
{
    return this->prom.get_future();
}

template<typename FunctionType, typename... ArgPack>
template<typename... ArgumentFwdTypes>
BoundFunction<void, FunctionType, ArgPack...>::BoundFunction(FunctionType func, ArgumentFwdTypes&&... args)
:func(func), args( std::forward<ArgumentFwdTypes>(args)...)
{}

template<typename ReturnType, typename... ParamTypes, typename... ArgumentFwdTypes>
auto make_bound_function(ReturnType(func)(ParamTypes...), ArgumentFwdTypes&&... args)
{
    auto shortened_function = shorten_function_paramlist(func, std::forward<ArgumentFwdTypes>(args)...);
    return BoundFunction<ReturnType, decltype(shortened_function), ArgumentFwdTypes...>(shortened_function, std::forward<ArgumentFwdTypes>(args)...);
}
