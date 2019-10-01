#ifndef TUPLEPP_H
#define TUPLEPP_H

#include <tuple>

namespace epp
{

template<class T, class... Pack>
inline constexpr bool isTypeInPack()
{
    return false || (std::is_same_v<T, Pack> || ...);
}


template<class... TplTypes>
struct TuplePP : public std::tuple<TplTypes...>
{
    using ThisTpl_t = TuplePP<TplTypes...>;

    using Base_t = std::tuple<TplTypes...>;


    TuplePP() = default;

    TuplePP(ThisTpl_t const&) = default;

    TuplePP(Base_t const& rhs)
        : Base_t(rhs)
    {}


    template<class U>
    U& get();

    template<class U>
    U const& get() const;

    template<std::size_t i>
    decltype(auto) get();

    template<std::size_t i>
    decltype(auto) get() const;


    template<class... OtherTypes>
    TuplePP<OtherTypes...> asTuple();

    template<class... OtherTypes>
    TuplePP<OtherTypes...> asTuple() const;


    TuplePP<TplTypes&...> asRefTuple();

    TuplePP<TplTypes const&...> asRefTuple() const;


    template<class... OtherTypes>
    TuplePP<OtherTypes&...> asRefTuple();

    template<class... OtherTypes>
    TuplePP<OtherTypes const&...> asRefTuple() const;


    template<class... OtherTplTypes>
    static ThisTpl_t makeFromTuple(TuplePP<OtherTplTypes...> const& tplToCpyFrom)
    {
        return ThisTpl_t(Base_t(tplToCpyFrom.template get<TplTypes>()...));
    }

    template<class... SearchedTypes>
    static constexpr bool containsType()
    {
        if constexpr (sizeof...(SearchedTypes) > 0)
            return (isTypeInPack<SearchedTypes, TplTypes...>() && ...);
        else
            return false;
    }
};

#include <ECSpp/Utility/TuplePP.inl>

} // namespace epp

#endif // TUPLEPP_H