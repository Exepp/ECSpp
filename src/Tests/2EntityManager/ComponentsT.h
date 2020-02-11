#ifndef EPP_COMPONENTS_H
#define EPP_COMPONENTS_H

#include <ECSpp/Component.h>
#include <array>

template <typename T>
struct TCompBase {
    using Base_t = TCompBase<T>;
    using Arr_t = std::array<int, 3>;
    TCompBase() { ++AliveCounter; }
    TCompBase(Arr_t arr) : data(arr) { ++AliveCounter; }
    TCompBase(Base_t&& rval) : data(rval.data) { ++AliveCounter; }
    TCompBase(Base_t const&) = delete;
    ~TCompBase() { --AliveCounter; }

    bool operator==(Base_t const& rhs) const { return data == rhs.data; }
    bool operator!=(Base_t const& rhs) const { return !(*this == rhs); }

    inline static int AliveCounter = 0;

    alignas(256) Arr_t data = { 0, 0, 0 };
    std::vector<int> dynamicData = std::vector<int>(10);
};

struct TComp1 : public TCompBase<TComp1> {
    TComp1() = default;
    TComp1(int a, float b, double c) : a(a), b(b), c(c) {}
    TComp1(TComp1 const&) = delete;
    TComp1(TComp1&& other)
    {
        a = other.a;
        b = other.b;
        c = other.c;
        other.a = 0;
        other.b = 0.f;
        other.c = 0.0;
    }

    TComp1 copy(int n = 0)
    {
        TComp1 cpy;
        cpy.a = a + n;
        cpy.b = b + n;
        cpy.c = c + n;
        return cpy;
    }

    bool operator==(TComp1 const& rhs) const { return a == rhs.a && b == rhs.b && c == rhs.c && Base_t::operator==(rhs); }
    bool operator!=(TComp1 const& rhs) const { return !(*this == rhs); }

    alignas(512) int a = 1;
    float b = 1.25f;
    double c = 3.0;
};
struct TComp2 : public TCompBase<TComp2> {
    TComp2() = default;
    TComp2(Arr_t data) : Base_t(data) {}
};
struct TComp3 : public TCompBase<TComp3> {
    TComp3() = default;
    TComp3(Arr_t data) : Base_t(data) {}
};
struct TComp4 : public TCompBase<TComp4> {
    TComp4() = default;
    TComp4(Arr_t data) : Base_t(data) {}
};

#endif // EPP_COMPONENTS_H