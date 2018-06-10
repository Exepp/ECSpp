
template<class ...TplTypes>
template<class U>
inline U& TuplePP<TplTypes...>::get()
{
	static_assert(isTypeInPack<U, TplTypes...>(), "Tried to get a wrong type from a tuple!");
	return std::get<U>(*this);
}

template<class ...TplTypes>
template<class U>
inline const U& TuplePP<TplTypes...>::get() const
{
	static_assert(isTypeInPack<U, TplTypes...>(), "Tried to get a wrong type from a tuple!");
	return std::get<U>(*this);
}

template<class ...TplTypes>
template<size_t i>
inline decltype(auto) TuplePP<TplTypes...>::get()
{
	static_assert(i < sizeof...(TplTypes), "Tried to get a wrong type from a tuple!");
	return std::get<i>(*this);
}

template<class ...TplTypes>
template<size_t i>
inline decltype(auto) TuplePP<TplTypes...>::get() const
{
	static_assert(i < sizeof...(TplTypes), "Tried to get a wrong type from a tuple!");
	return std::get<i>(*this);
}

template<class ...TplTypes>
template<class ...OtherTypes>
inline TuplePP<OtherTypes...> TuplePP<TplTypes...>::asTuple()
{
	static_assert(AreUsed<OtherTypes...>::value || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
	return std::tuple<OtherTypes...>(get<OtherTypes>()...);
}


template<class ...TplTypes>
template<class ...OtherTypes>
inline TuplePP<OtherTypes...> TuplePP<TplTypes...>::asTuple() const
{
	static_assert(AreUsed<OtherTypes...>::value || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
	return std::tuple<OtherTypes...>(get<OtherTypes>()...);
}



template<class ...TplTypes>
inline TuplePP<TplTypes&...> TuplePP<TplTypes...>::asRefTuple()
{
	return std::tuple<TplTypes&...>(get<TplTypes>()...);
}


template<class ...TplTypes>
inline TuplePP<const TplTypes&...> TuplePP<TplTypes...>::asRefTuple() const
{
	return std::tuple<const TplTypes&...>(get<TplTypes>()...);
}



template<class ...TplTypes>
template<class ...OtherTypes>
inline TuplePP<OtherTypes&...> TuplePP<TplTypes...>::asRefTuple()
{
	static_assert(AreUsed<OtherTypes...>::value || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
	return std::tuple<OtherTypes&...>(get<OtherTypes>()...);
}


template<class ...TplTypes>
template<class ...OtherTypes>
inline TuplePP<const OtherTypes&...> TuplePP<TplTypes...>::asRefTuple() const
{
	static_assert(AreUsed<OtherTypes...>::value || !sizeof...(OtherTypes), "Tuple does not contain given sequence of types");
	return std::tuple<const OtherTypes&...>(get<OtherTypes>()...);
}