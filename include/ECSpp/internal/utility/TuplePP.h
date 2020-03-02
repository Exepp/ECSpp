#ifndef EPP_TUPLEPP_H
#define EPP_TUPLEPP_H

#include <tuple>

namespace epp {

template <typename T, typename... Pack>
inline constexpr bool isTypeInPack()
{
    return false || (std::is_same_v<T, Pack> || ...);
}


template <typename... TplTypes>
struct TuplePP : public std::tuple<TplTypes...> {

    using Base_t = std::tuple<TplTypes...>;


    TuplePP() = default;

    TuplePP(TuplePP const&) = default;

    TuplePP(Base_t const& rhs) : Base_t(rhs) {}


    template <typename U>
    U& get()
    {
        static_assert(isTypeInPack<U, TplTypes...>(), "Tried to get a wrong type from a tuple!");
        return std::get<U>(*this);
    }

    template <typename U>
    U const& get() const
    {
        static_assert(isTypeInPack<U, TplTypes...>(), "Tried to get a wrong type from a tuple!");
        return std::get<U>(*this);
    }

    template <std::size_t i>
    decltype(auto) get()
    {
        static_assert(i < sizeof...(TplTypes), "Tried to get a wrong type from a tuple!");
        return std::get<i>(*this);
    }

    template <std::size_t i>
    decltype(auto) get() const
    {
        static_assert(i < sizeof...(TplTypes), "Tried to get a wrong type from a tuple!");
        return std::get<i>(*this);
    }


    template <typename... OtherTypes>
    TuplePP<OtherTypes...> asTuple()
    {
        static_assert(containsType<OtherTypes...>() || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
        return std::tuple<OtherTypes...>(get<OtherTypes>()...);
    }

    template <typename... OtherTypes>
    TuplePP<OtherTypes...> asTuple() const
    {
        static_assert(containsType<OtherTypes...>() || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
        return std::tuple<OtherTypes...>(get<OtherTypes>()...);
    }

    TuplePP<TplTypes&...> asRefTuple()
    {
        return std::tuple<TplTypes&...>(get<TplTypes>()...);
    }

    TuplePP<TplTypes const&...> asRefTuple() const
    {
        return std::tuple<const TplTypes&...>(get<TplTypes>()...);
    }

    template <typename... OtherTypes>
    TuplePP<OtherTypes&...> asRefTuple()
    {
        static_assert(containsType<OtherTypes...>() || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
        return std::tuple<OtherTypes&...>(get<OtherTypes>()...);
    }

    template <typename... OtherTypes>
    TuplePP<OtherTypes const&...> asRefTuple() const
    {
        static_assert(containsType<OtherTypes...>() || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
        return std::tuple<const OtherTypes&...>(get<OtherTypes>()...);
    }


    template <typename... OtherTplTypes>
    static TuplePP makeFromTuple(TuplePP<OtherTplTypes...> const& tplToCpyFrom)
    {
        return TuplePP(Base_t(tplToCpyFrom.template get<TplTypes>()...));
    }

    template <typename... SearchedTypes>
    static constexpr bool containsType()
    {
        if constexpr (sizeof...(SearchedTypes) > 0)
            return (isTypeInPack<SearchedTypes, TplTypes...>() && ...);
        else
            return true;
    }
};

} // namespace epp

#endif // EPP_TUPLEPP_H