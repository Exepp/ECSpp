#include "EntityManager/ComponentsT.h"

#include <ECSpp/EntityManager/EntityManager.h>
#include <gtest/gtest.h>


using namespace epp;


TEST(Interface, T1)
{
    EntityManager mgr;
    Archetype     arche(IDOfComp<TComp1>());
    auto          entity = mgr.spawn();
}