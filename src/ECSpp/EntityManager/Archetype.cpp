#include <ECSpp/EntityManager/Archetype.h>

using namespace epp;

Archetype::Archetype(IDList_t initList)
{
    addComponent(initList);
}

void Archetype::addComponent(IDList_t ids)
{
    for (auto id : ids)
        addComponent(id);
}

void Archetype::addComponent(ComponentID id)
{
    if (cMask.get(id))
        return;
    cMask.set(id);
    creators.push_back({ id, ComponentUtility::GetCPoolsFactory()[id] });
}

void Archetype::removeComponent(IDList_t ids)
{
    for (auto id : ids)
        removeComponent(id);
}

void Archetype::removeComponent(ComponentID id)
{
    if (!has(id))
        return;
    cMask.unset(id);
    creators.erase(std::find_if(creators.begin(), creators.end(),
                                [id](auto const& creator) { return creator.id == id; }));
}

void Archetype::reset()
{
    cMask.clear();
    creators.clear();
}

bool Archetype::hasAllOf(IDList_t ids) const
{
    bool result = true;
    for (auto id : ids)
        result &= has(id);
    return result;
}

bool Archetype::hasAnyOf(IDList_t ids) const
{
    bool result = false;
    for (auto id : ids)
        result |= has(id);
    return result;
}

bool Archetype::has(ComponentID id) const
{
    return cMask.get(id);
}


Bitmask const& Archetype::getMask() const
{
    return cMask;
}