#include "Utility/Bitmask.h"
#include <bitset>

Bitmask::Bitmask(std::initializer_list<size_t> list)
{
	for (auto& index : list)
		set(index);
}

Bitmask& Bitmask::set(size_t bitIndex)
{
	size_t index = bitIndex / maskBits;
	if (index >= masks.size())
		masks.resize(index + 1);
	Bitmask::Mask_t before = masks[index];
	masks[index] |= 1ull << bitIndex % maskBits;

	if (before != masks[index])
	{
		++setCount;
		rehash = true;
	}
	return *this;
}

Bitmask& Bitmask::unset(size_t bitIndex)
{
	size_t index = bitIndex / maskBits;
	if (index < masks.size())
	{
		Bitmask::Mask_t before = masks[index];
		masks[index] &= ~(1ull << bitIndex % maskBits);

		if(before != masks[index])
			--setCount;

		rehash = true;
	}
	return *this;
}

bool Bitmask::get(size_t bitIndex) const
{
	size_t index = bitIndex / maskBits;
	if (index >= masks.size())
		return false;
	return (masks[index] & (1ull << (bitIndex % maskBits)));
}

void Bitmask::clear()
{
	masks.clear();
	setCount = 0;
	rehash = true;
	hashValue = 0;
}

Bitmask& Bitmask::flip()
{
	setCount = masks.size() * sizeof(Mask_t) * 8 - setCount;
	for (auto& mask : masks)
		mask = ~mask;
	rehash = true;
	return *this;
}

Bitmask Bitmask::flipped() const
{
	return Bitmask(*this).flip();
}

size_t Bitmask::getSetCount() const
{
	return setCount;
}

bool Bitmask::hasCommon(const Bitmask & other) const
{
	size_t smallest = std::_Min_value(masks.size(), other.masks.size());
	for (size_t i = 0; i < smallest; i++)
		if (masks[i] & other.masks[i])
			return true;
	return false;
}

size_t Bitmask::numberOfCommon(const Bitmask & other) const
{
	size_t smallest = std::_Min_value(masks.size(), other.masks.size());
	size_t sum = 0;
	std::bitset<sizeof(Mask_t) * 8u> counter;

	for (size_t i = 0; i < smallest; i++)
			sum += (counter = (masks[i] & other.masks[i])).count();
	return sum;
}

size_t Bitmask::hash() const
{
	if (rehash)
	{
		rehash = false;
		hashValue = std::hash<Bitmask>()(*this);
	}
	return hashValue;
}

Bitmask Bitmask::operator&(const Bitmask & rhs) const
{
	Bitmask result;
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());
	result.masks.resize(smallest);

	std::bitset<sizeof(Mask_t) * 8u> counter;
	for (size_t i = 0; i < smallest; i++)
		if (result.masks[i] = masks[i] & rhs.masks[i])
			result.setCount += (counter = result.masks[i]).count();

	return result;
}

Bitmask Bitmask::operator|(const Bitmask & rhs) const
{
	Bitmask result;
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());

	result.masks.resize(smallest);

	std::bitset<sizeof(Mask_t) * 8u> counter;
	for (size_t i = 0; i < smallest; i++)
		if (result.masks[i] = masks[i] | rhs.masks[i])
			result.setCount += (counter = result.masks[i]).count();

	return result;
}

Bitmask & Bitmask::operator&=(const Bitmask & rhs)
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());

	if (masks.size() > smallest)
		masks.resize(smallest);

	setCount = 0;
	std::bitset<sizeof(Mask_t) * 8u> counter;
	for (size_t i = 0; i < smallest; i++)
		if (masks[i] &= rhs.masks[i])
			setCount += (counter = masks[i]).count();
	rehash = true;
	return *this;
}

Bitmask & Bitmask::operator|=(const Bitmask & rhs)
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());

	setCount = 0;
	std::bitset<sizeof(Mask_t) * 8u> counter;
	for (size_t i = 0; i < smallest; i++)
		if (masks[i] |= rhs.masks[i])
			setCount += (counter = masks[i]).count();
	rehash = true;
	return *this;
}

bool Bitmask::operator==(const Bitmask & rhs) const
{
	return setCount == rhs.setCount && (setCount == 0 || masks == rhs.masks);
}

bool Bitmask::operator!=(const Bitmask & rhs) const
{
	return !(*this == rhs);
}