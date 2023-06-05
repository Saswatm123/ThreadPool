#ifndef FUNC_TOOLS_HPP
#define FUNC_TOOLS_HPP

template<typename... ArgPack>
struct TypeHolder {};

// Duplicate of tuple_cat
template<typename... LeftTypeHolderTypes, typename... RightTypeHolderTypes>
TypeHolder<LeftTypeHolderTypes..., RightTypeHolderTypes...> TypeHolder_cat(TypeHolder<LeftTypeHolderTypes...>, TypeHolder<RightTypeHolderTypes...>);

// FirstNTypes takes in a size_t as its first template argument, and a param pack of types. It returns a
// TypeHolder<Type_1, Type_2, ... Type_N> under the ::type specifier
template<std::size_t N, typename... ArgPack>
struct FirstNTypes {
    static_assert(N == 0,
        "N requested was too large for number of args in ArgPack. Number of args hit zero before N did. N should always be <= number of args in ArgPack."
    );

    using type = TypeHolder<>;
};

template<std::size_t N, typename FirstArg, typename... ArgPack>
struct FirstNTypes<N, FirstArg, ArgPack...>
{
    template<std::size_t NCheck, typename... Args>
    struct CheckIfNIsZero {
        using type = decltype(TypeHolder_cat( std::declval<TypeHolder<FirstArg> >(), std::declval< typename FirstNTypes<NCheck-1, ArgPack...>::type >() ) );
    };

    template<typename... Args>
    struct CheckIfNIsZero<0, Args...> {
        using type = TypeHolder<>;
    };

    using type = typename CheckIfNIsZero<N, ArgPack...>::type;
};

template<typename ReturnType, typename... TruncatedParamTypes, typename... ParamTypes, typename... ArgTypes>
ReturnType(*_shorten_function_paramlist_impl(TypeHolder<TruncatedParamTypes...>, ReturnType(function)(ParamTypes...), ArgTypes&&... args) )(TruncatedParamTypes...)
{
    return reinterpret_cast<ReturnType(*)(TruncatedParamTypes...)>(function);
}

template<typename ReturnType, typename... ParamTypes, typename... ArgTypes>
auto shorten_function_paramlist(ReturnType(function)(ParamTypes...), ArgTypes&&... args)
{
    return _shorten_function_paramlist_impl(
        typename FirstNTypes<sizeof...(args), ParamTypes...>::type(),
        function,
        std::forward<ArgTypes>(args)...
    );
}

#endif //FUNC_TOOLS_HPP
