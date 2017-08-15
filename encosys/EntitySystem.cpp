#include "EntitySystem.h"

#include <algorithm>
#include <cassert>
#include "ComponentTypeRegistry.h"

namespace ecs {

EntitySystem::EntitySystem (const ComponentTypeRegistry& componentRegistry) :
    m_componentRegistry{componentRegistry} {
}

Entity EntitySystem::Create () {
    Entity entity(m_entityIdCounter);
    const uint32_t index = static_cast<uint32_t>(m_componentIndexCards.size());
    m_entityToIndexCard[entity] = index;
    m_componentIndexCards.resize(index + 1);
    std::fill(m_componentIndexCards[index].begin(), m_componentIndexCards[index].end(), c_invalidIndex);
    ++m_entityIdCounter;
    return entity;
}

uint8_t* EntitySystem::Add (Entity e, ComponentTypeIndex typeIndex) {
    // Verify this entity exists
    auto indexCard = m_entityToIndexCard.find(e);
    assert(indexCard != m_entityToIndexCard.end());

    // Retrieve the registered type of the component
    const ComponentType& type = m_componentRegistry.GetType(typeIndex);

    // Increase the storage capacity for this component type and return
    // a pointer to the component's memory for external initialization.
    ComponentByteVector& storage = m_componentByteVectors[typeIndex];
    std::size_t byteIndex = storage.size();
    m_componentIndexCards[indexCard->second][typeIndex] = static_cast<ComponentIndex>(byteIndex);
    storage.resize(byteIndex + type.m_bytes);
    return &storage[byteIndex];
}

uint8_t* EntitySystem::Get (Entity e, ComponentTypeIndex typeIndex) {
    // Verify this entity exists
    auto indexCard = m_entityToIndexCard.find(e);
    assert(indexCard != m_entityToIndexCard.end());

    // Retrieve the registered type of the component
    const ComponentType& type = m_componentRegistry.GetType(typeIndex);

    // Return a pointer to the component's memory.
    ComponentByteVector& storage = m_componentByteVectors[typeIndex];
    ComponentIndex byteIndex = m_componentIndexCards[indexCard->second][typeIndex];
    return &storage[byteIndex];
}

} // namespace ecs
