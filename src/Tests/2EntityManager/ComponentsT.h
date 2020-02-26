#ifndef EPP_COMPONENTS_H
#define EPP_COMPONENTS_H

#include <ECSpp/Component.h>
#include <array>
#include <vector>

template <int N>
struct TCompBase {
    using Base_t = TCompBase<N>;
    using Arr_t = std::array<int, 3>;
    TCompBase() { ++AliveCounter; }
    TCompBase(Arr_t arr) : data(arr) { ++AliveCounter; }
    TCompBase(Base_t&& rval) : data(rval.data) { ++AliveCounter; }
    TCompBase(Base_t const& other) : TCompBase(other.data) {}
    TCompBase& operator=(TCompBase const& other)
    {
        Base_t::~Base_t();
        new (this) TCompBase(other);
        return *this;
    }
    TCompBase& operator=(TCompBase&& other)
    {
        Base_t::~Base_t();
        new (this) TCompBase(std::move(other));
        return *this;
    }
    ~TCompBase() { --AliveCounter; }

    bool operator==(Base_t const& rhs) const { return data == rhs.data; }
    bool operator!=(Base_t const& rhs) const { return !(*this == rhs); }

    inline static int AliveCounter = 0;

    alignas(256) Arr_t data = { 0, 0, 0 };
    std::vector<int> dynamicData = std::vector<int>(10);
};

template <>
struct TCompBase<1> : public TCompBase<0> {
    TCompBase(Arr_t arr = {}) : Base_t(arr), a(arr[0]), b(arr[1]), c(arr[2]) {}
    TCompBase(TCompBase const& other) : Base_t(other), a(other.a), b(other.b), c(other.c) {}
    TCompBase(TCompBase&& other) : Base_t(std::move(other)), a(other.a), b(other.b), c(other.c)
    {
        other.a = 0;
        other.b = 0.f;
        other.c = 0.0;
    }
    TCompBase& operator=(TCompBase const& other)
    {
        this->~TCompBase();
        new (this) TCompBase(other);
        return *this;
    }
    TCompBase& operator=(TCompBase&& other)
    {
        this->~TCompBase();
        new (this) TCompBase(std::move(other));
        return *this;
    }

    bool operator==(TCompBase const& rhs) const
    {
        return a == rhs.a && b == rhs.b && c == rhs.c && Base_t::operator==(rhs);
    }
    bool operator!=(TCompBase const& rhs) const
    {
        return !(*this == rhs);
    }

    alignas(512) int a = 0;
    float b = 0.f;
    double c = 0.0;
};

using TComp1 = TCompBase<1>;
using TComp2 = TCompBase<2>;
using TComp3 = TCompBase<3>;
using TComp4 = TCompBase<4>;

#endif // EPP_COMPONENTS_H