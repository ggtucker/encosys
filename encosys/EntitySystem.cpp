#include "EntitySystem.h"

#include <algorithm>
#include <cassert>
#include "ComponentTypeRegistry.h"

namespace ecs {

EntitySystem::EntitySystem (ComponentTypeRegistry& componentRegistry) :
    m_componentRegistry{componentRegistry} {
}

EntityId EntitySystem::Create (bool active) {
    EntityId id(m_entityIdCounter);
    ++m_entityIdCounter;

    if (active) {
        if (m_entityActiveCount == EntityCount()) {
            m_idToEntity[id] = EntityCount();
            m_entities.push_back(Entity(id));
        }
        else {
            const Entity& firstInactiveEntity = m_entities[m_entityActiveCount];
            m_idToEntity[firstInactiveEntity.GetId()] = EntityCount();
            m_entities.push_back(firstInactiveEntity);
            m_idToEntity[id] = m_entityActiveCount;
            m_entities[m_entityActiveCount] = Entity(id);
        }
        ++m_entityActiveCount;
    }
    else {
        m_idToEntity[id] = EntityCount();
        m_entities.push_back(Entity(id));
    }

    return id;
}

void EntitySystem::Destroy (EntityId e) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Cache off the information about this entity
    uint32_t entityIndex = entityIter->second;
    Entity& entity = m_entities[entityIndex];

    // Destroy the components for this entity
    for (uint32_t i = 0; i < m_componentRegistry.Count(); ++i) {
        const ComponentTypeId typeId = m_componentRegistry[i].m_id;
        if (entity.HasComponent(typeId)) {
            auto& storage = m_componentRegistry.GetStorage(typeId);
            storage.Destroy(entity.GetComponentIndex(typeId));
            entity.RemoveComponentIndex(typeId);
        }
    }

    // Move the entity to the end of the vector and erase it
    IndexSetActive(entityIndex, false);
    IndexSwapEntities(entityIndex, EntityCount() - 1);
    m_idToEntity.erase(entityIter);
    m_entities.pop_back();
}

bool EntitySystem::IsValid (EntityId e) const {
    return m_idToEntity.find(e) != m_idToEntity.end();
}

bool EntitySystem::IsActive (EntityId e) const {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    return IndexIsActive(entityIter->second);
}

void EntitySystem::SetActive (EntityId e, bool active) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    uint32_t entityIndex = entityIter->second;
    IndexSetActive(entityIndex, active);
}

uint32_t EntitySystem::EntityCount () const {
    return static_cast<uint32_t>(m_entities.size());
}

void EntitySystem::IndexSwapEntities (uint32_t lhsIndex, uint32_t rhsIndex) {
    if (lhsIndex == rhsIndex) {
        return;
    }
    m_idToEntity[m_entities[lhsIndex].GetId()] = rhsIndex;
    m_idToEntity[m_entities[rhsIndex].GetId()] = lhsIndex;
    std::swap(m_entities[lhsIndex], m_entities[rhsIndex]);
}

bool EntitySystem::IndexIsActive (uint32_t index) const {
    return index < m_entityActiveCount;
}

void EntitySystem::IndexSetActive (uint32_t& index, bool active) {
    if (active == IndexIsActive(index)) {
        return;
    }
    const uint32_t newIndex = active ? m_entityActiveCount : m_entityActiveCount - 1;
    IndexSwapEntities(index, newIndex);
    m_entityActiveCount += (active ? 1 : -1);
    index = newIndex;
}

} // namespace ecs
