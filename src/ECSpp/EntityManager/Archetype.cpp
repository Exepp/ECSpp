#include <ECSpp/Internal/Archetype.h>

using namespace epp;

Archetype::Archetype(IdList_t initList)
{
    addComponent(initList);
}

Archetype& Archetype::addComponent(IdList_t ids)
{
    for (auto id : ids)
        addComponent(id);
    return *this;
}

Archetype& Archetype::addComponent(ComponentId id)
{
    if (!cMask.get(id)) {
        cMask.set(id);
        cIds.push_back(id);
    }
    return *this;
}

Archetype& Archetype::removeComponent(IdList_t ids)
{
    for (auto id : ids)
        removeComponent(id);
    return *this;
}

Archetype& Archetype::removeComponent(ComponentId id)
{
    if (has(id)) {
        cMask.unset(id);
        cIds.erase(std::find_if(cIds.begin(), cIds.end(), [id](auto const& cId) { return cId == id; }));
    }
    return *this;
}

void Archetype::reset()
{
    cMask.clear();
    cIds.clear();
}

bool Archetype::hasAllOf(IdList_t ids) const
{
    return cMask.contains(ids);
}

bool Archetype::hasAnyOf(IdList_t ids) const
{
    return cMask.hasCommon(ids);
}

bool Archetype::has(ComponentId id) const
{
    return cMask.get(id);
}