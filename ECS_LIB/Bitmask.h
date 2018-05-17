#pragma once
#include <string>


class Bitmask
{
	using Mask_t = uint32_t;

	using MaskContainer_t = std::basic_string<uint32_t>;


public:

	void set(size_t bitIndex, bool value = true);
	

	Bitmask operator&(const Bitmask& rhs) const;

	Bitmask operator|(const Bitmask& rhs) const;


	Bitmask& operator&=(const Bitmask& rhs);

	Bitmask& operator|=(const Bitmask& rhs);


	bool operator==(const Bitmask& rhs) const;

	bool operator!=(const Bitmask& rhs) const;

	
	size_t hash() const;

private:

	static const uint8_t maskBits = sizeof(Mask_t) * 8u;

private:

	MaskContainer_t masks;

};

