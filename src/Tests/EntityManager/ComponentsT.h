#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <ECSpp/Component.h>
#include <ECSpp/EntityManager/ComponentUtility.h>

using namespace epp;

struct TComp1 : public Component
{
    TComp1()
    {
        int i = 0;
        for (auto y : test)
            for (auto& x : test)
                x = ++i;
    }
    int test[5];
};
struct TComp2 : public Component
{
    int test[5];
};
struct TComp3 : public Component
{
    int test[5];
};
struct TComp4 : public Component
{
    int test[5];
};

#endif // COMPONENTS_H