#pragma once

#include "BlockObjectPool.h"
#include "ComponentType.h"
#include "EncosysDefines.h"
#include <array>
#include <cassert>
#include <map>
#include <typeindex>

namespace ecs {

class ComponentRegistry {
public:
    virtual ~ComponentRegistry () {
        for (uint32_t i = 0; i < Count(); ++i) {
            delete m_componentPools[i];
        }
    }

    template <typename TComponent>
    ComponentTypeId Register () {
        using TDecayed = std::decay_t<TComponent>;
        const ComponentTypeId id = Count();
        assert(id < ENCOSYS_MAX_COMPONENTS_);
        assert(m_typeToId.find(typeid(TDecayed)) == m_typeToId.end());
        m_componentTypes[id] = ComponentType(id, sizeof(TDecayed));
        m_typeToId[typeid(TDecayed)] = id;

        m_componentPools[id] = new BlockObjectPool<TDecayed>();
        assert(m_componentPools[id] != nullptr);
        return id;
    }

    template <typename TComponent>
    ComponentTypeId GetTypeId () const {
        auto it = m_typeToId.find(typeid(std::decay_t<TComponent>));
        assert(it != m_typeToId.cend());
        return it->second;
    }

    template <typename TComponent>
    const ComponentType& GetType () const {
        return GetType(GetTypeId<TComponent>());
    }

    const ComponentType& GetType (ComponentTypeId id) const {
        assert(id < Count());
        return m_componentTypes[id];
    }

    bool HasType (ComponentTypeId id) const {
        return id < Count();
    }

    template <typename TComponent>
    bool HasType () {
        return HasType(GetTypeId<TComponent>());
    }

    template <typename TComponent>
    auto& GetStorage () { return static_cast<BlockObjectPool<std::decay_t<TComponent>>&>(GetStorage(GetTypeId<TComponent>())); }

    template <typename TComponent>
    const auto& GetStorage () const { return static_cast<const BlockObjectPool<std::decay_t<TComponent>>&>(GetStorage(GetTypeId<TComponent>())); }

    BlockMemoryPool& GetStorage (ComponentTypeId id) { assert(id < Count()); return *m_componentPools[id]; }
    const BlockMemoryPool& GetStorage (ComponentTypeId id) const { assert(id < Count()); return *m_componentPools[id]; }

    uint32_t Count () const { return static_cast<uint32_t>(m_typeToId.size()); }
    const ComponentType& operator[] (uint32_t index) const { return m_componentTypes[index]; }

private:
    std::array<BlockMemoryPool*, ENCOSYS_MAX_COMPONENTS_> m_componentPools{};
    std::array<ComponentType, ENCOSYS_MAX_COMPONENTS_> m_componentTypes;
    std::map<std::type_index, ComponentTypeId> m_typeToId{};
};

} // namespace ecs