#pragma once

#include "BlockObjectPool.h"
#include "EncosysDefines.h"
#include <array>
#include <cassert>
#include <map>
#include <typeindex>

namespace ecs {

struct ComponentType {
    ComponentType () {}
    ComponentType (ComponentTypeId id, const char* name, uint32_t bytes) :
        m_id{id},
        m_name{name},
        m_bytes{bytes} {
    }

    ComponentTypeId m_id{};
    const char* m_name{};
    uint32_t m_bytes{};
};

class ComponentTypeRegistry {
public:
    ~ComponentTypeRegistry () {
        for (uint32_t i = 0; i < Count(); ++i) {
            delete m_componentPools[i];
        }
    }

    template <typename TComponent>
    ComponentTypeId Register (const char* name) {
        using TDecayed = std::decay_t<TComponent>;
        const ComponentTypeId id = Count();
        assert(id < ENCOSYS_MAX_COMPONENTS_);
        assert(m_typeToId.find(typeid(TDecayed)) == m_typeToId.end());
        m_componentTypes[id] = ComponentType(
            id,
            name,
            sizeof(TDecayed)
        );
        m_componentPools[id] = new BlockObjectPool<TDecayed>();
        assert(m_componentPools[id] != nullptr);
        m_typeToId[typeid(TDecayed)] = id;
        return id;
    }

    template <typename TComponent>
    ComponentTypeId GetTypeId () const {
        auto it = m_typeToId.find(typeid(std::decay_t<TComponent>));
        assert(it != m_typeToId.cend());
        return it->second;
    }

    ComponentTypeId GetTypeId (const char* name) const {
        for (ComponentTypeId i = 0; i < Count(); ++i) {
            if (strcmp(m_componentTypes[i].m_name, name) == 0) {
                return i;
            }
        }
        return c_invalidIndex;
    }

    const ComponentType& GetType (ComponentTypeId id) const {
        assert(id < Count());
        return m_componentTypes[id];
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
    std::array<ComponentType, ENCOSYS_MAX_COMPONENTS_> m_componentTypes;
    std::array<BlockMemoryPool*, ENCOSYS_MAX_COMPONENTS_> m_componentPools{};
    std::map<std::type_index, ComponentTypeId> m_typeToId{};
};

} // namespace ecs