#pragma once
#include <tuple>

namespace epp
{

	template<class T, class ...Pack>
	inline constexpr bool isTypeInPack()
	{
		return (std::is_same_v<T, Pack> || ...);
	}


	template<class ...TplTypes>
	struct TuplePP : public std::tuple<TplTypes...>
	{
		using ThisTpl_t = TuplePP<TplTypes...>;

		using Base_t = std::tuple<TplTypes...>;


		TuplePP() = default;

		TuplePP(const ThisTpl_t&) = default;

		TuplePP(const Base_t& rhs) : Base_t(rhs) {}


		template<class U>
		U& get();

		template<class U>
		const U& get() const;

		template<size_t i>
		decltype(auto) get();

		template<size_t i>
		decltype(auto) get() const;


		template<class ...OtherTypes>
		TuplePP<OtherTypes...> asTuple();

		template<class ...OtherTypes>
		TuplePP<OtherTypes...> asTuple() const;


		TuplePP<TplTypes&...> asRefTuple();

		TuplePP<const TplTypes&...> asRefTuple() const;


		template<class ...OtherTypes>
		TuplePP<OtherTypes&...> asRefTuple();

		template<class ...OtherTypes>
		TuplePP<const OtherTypes&...> asRefTuple() const;


		template<class ...OtherTplTypes>
		static ThisTpl_t makeFromTuple(const TuplePP<OtherTplTypes...>& tplToCpyFrom)
		{
			return ThisTpl_t(Base_t(tplToCpyFrom.get<TplTypes>()...));
		}

		template<class ...SearchedTypes>
		static constexpr bool containsType()
		{
			return (isTypeInPack<SearchedTypes, TplTypes...>() && ...);
		}

		template<>
		static constexpr bool containsType<>()
		{
			return false;
		}


		template<class ...OtherTypes>
		struct AreUsed : public std::bool_constant<containsType<OtherTypes...>()> {};

	};

#include "TuplePP.inl"

}