#pragma once

#include "EncosysDefines.h"
#include "Entity.h"
#include <array>
#include <bitset>
#include <unordered_map>
#include <vector>

namespace ecs {

// Forward declarations
class ComponentTypeRegistry;

using ComponentIndex = uint32_t;
using ComponentIndexCard = std::array<ComponentIndex, ENCOSYS_MAX_COMPONENTS_>;
using ComponentByteVector = std::vector<uint8_t>;

class EntitySystem {
public:
    explicit EntitySystem (const ComponentTypeRegistry& componentRegistry);

    Entity Create ();

    template <typename TComponent>
    TComponent& Add (Entity e) { return *reinterpret_cast<TComponent*>(Add(e, m_componentRegistry.GetTypeIndex<TComponent>())); }
    uint8_t* Add (Entity e, ComponentTypeIndex typeIndex);

    template <typename TComponent>
    TComponent& Get (Entity e) { return *reinterpret_cast<TComponent*>(Get(e, m_componentRegistry.GetTypeIndex<TComponent>())); }
    uint8_t* Get (Entity e, ComponentTypeIndex typeIndex);


private:
    const ComponentTypeRegistry& m_componentRegistry;

    std::unordered_map<Entity, uint32_t> m_entityToIndexCard;
    std::vector<ComponentIndexCard> m_componentIndexCards;
    std::array<ComponentByteVector, ENCOSYS_MAX_COMPONENTS_> m_componentByteVectors;
    uint64_t m_entityIdCounter{};
};

} // namespace ecs
