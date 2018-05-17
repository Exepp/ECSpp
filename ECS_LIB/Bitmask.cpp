#include "BitMask.h"


void Bitmask::set(size_t bitIndex, bool value)
{
	size_t index = bitIndex / maskBits;
	if(index + 1 > masks.size())
		masks.resize(index + 1);
	if(value)
		masks[index] |= 1ull << bitIndex % maskBits;
	else
		masks[index] &= ~(1ull << bitIndex % maskBits);
}

Bitmask Bitmask::operator&(const Bitmask & rhs) const
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());
	Bitmask result;

	result.masks.resize(smallest);

	for (size_t i = 0; i < smallest; i++)
		result.masks[i] = masks[i] & rhs.masks[i];

	return std::move(result);
}

Bitmask Bitmask::operator|(const Bitmask & rhs) const
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());
	Bitmask result;

	result.masks.resize(smallest);

	for (size_t i = 0; i < smallest; i++)
		result.masks[i] = masks[i] | rhs.masks[i];

	return std::move(result);
}

Bitmask & Bitmask::operator&=(const Bitmask & rhs)
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());

	if (masks.size() > smallest)
		masks.resize(smallest);

	for (size_t i = 0; i < smallest; i++)
		masks[i] &= rhs.masks[i];

	return *this;
}

Bitmask & Bitmask::operator|=(const Bitmask & rhs)
{
	size_t smallest = std::_Min_value(masks.size(), rhs.masks.size());

	for (size_t i = 0; i < smallest; i++)
		masks[i] |= rhs.masks[i];

	return *this;
}

bool Bitmask::operator==(const Bitmask & rhs) const
{
	return masks == rhs.masks;
}

bool Bitmask::operator!=(const Bitmask & rhs) const
{
	return !(*this == rhs);
}

size_t Bitmask::hash() const
{
	return std::hash<MaskContainer_t>()(masks);
}
