template<typename ReturnType, typename... ArgPack>
inline void BoundFunction<ReturnType, ArgPack...>::execute()
{
    this->prom.set_value( this->execute_impl(std::index_sequence_for<ArgPack...>() ) );
}

template<typename ReturnType, typename... ArgPack>
template<std::size_t... indices>
ReturnType BoundFunction<ReturnType, ArgPack...>::execute_impl(std::index_sequence<indices...>)
{
    return this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
}

template<typename ReturnType, typename... ArgPack>
std::future<ReturnType> BoundFunction<ReturnType, ArgPack...>::get_future()
{
    return this->prom.get_future();
}

template<typename ReturnType, typename... ArgPack>
template<typename... ArgumentFwdTypes>
BoundFunction<ReturnType, ArgPack...>::BoundFunction(ReturnType(*const func)(ArgPack...), ArgumentFwdTypes... args)
:func(func), args( std::forward<ArgumentFwdTypes>(args)... )
{}

template<typename... ArgPack>
template<std::size_t... indices>
void BoundFunction<void, ArgPack...>::execute_impl(std::index_sequence<indices...>)
{
    this->func(std::forward<ArgPack>(std::get<indices>(this->args) )...);
}

template<typename... ArgPack>
inline void BoundFunction<void, ArgPack...>::execute()
{
    this->execute_impl(std::index_sequence_for<ArgPack...>() );
}

template<typename... ArgPack>
std::future<void> BoundFunction<void, ArgPack...>::get_future()
{
    return this->prom.get_future();
}

template<typename... ArgPack>
template<typename... ArgumentFwdTypes>
BoundFunction<void, ArgPack...>::BoundFunction(void(*const func)(ArgPack...), ArgumentFwdTypes&&... args)
:func(func), args( std::forward<ArgumentFwdTypes>(args)...)
{}

template<typename ReturnType, typename... ArgPack, typename... ArgumentFwdTypes>
BoundFunction<ReturnType, ArgPack...> make_bound_function(ReturnType(*const func)(ArgPack...), ArgumentFwdTypes&&... args)
{
    return BoundFunction<ReturnType, ArgPack...>(func, std::forward<ArgumentFwdTypes>(args)...);
}
