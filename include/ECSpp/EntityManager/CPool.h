#ifndef CPOOL_H
#define CPOOL_H

#include <ECSpp/Component.h>
#include <ECSpp/Utility/Pool.h>

namespace epp
{

struct CPoolBase
{
    virtual ~CPoolBase() = default;

    virtual Component& alloc() = 0;

    virtual Component& alloc(Component&& arg) = 0;


    virtual void free(std::size_t i) = 0;


    virtual void prepareToFitNMore(std::size_t n) = 0;

    virtual void clear() = 0;


    virtual Component& operator[](std::size_t i) = 0;
};


template<class T>
struct CPool : public CPoolBase
{
    static_assert(std::is_base_of_v<Component, T>, "Use only component types in CPools");


    virtual Component& alloc() override;

    virtual Component& alloc(Component&& arg) override;


    virtual void free(std::size_t i) override;


    virtual void prepareToFitNMore(std::size_t n) override;

    virtual void clear() override;


    virtual Component& operator[](std::size_t i) override;


    Pool<T> pool;
};


#include <ECSpp/EntityManager/CPool.inl>

} // namespace epp

#endif // CPOOL_H