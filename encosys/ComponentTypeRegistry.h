#pragma once

#include "EncosysDefines.h"
#include <array>
#include <cassert>
#include <map>
#include <typeindex>

namespace ecs {

struct ComponentType {
    ComponentType () {}
    ComponentType (const char* name, uint32_t bytes) : m_name{name}, m_bytes {bytes} {}

    const char* m_name{};
    uint32_t m_bytes{};
};

class ComponentTypeRegistry {
public:
    template <typename TComponent>
    ComponentTypeIndex Register (const char* name) {
        const ComponentTypeIndex id = Count();
        assert(id < ENCOSYS_MAX_COMPONENTS_);
        assert(m_typeToId.find(typeid(TComponent)) == m_typeToId.end());
        m_componentTypes[id] = ComponentType(name, sizeof(TComponent));
        m_typeToId[typeid(TComponent)] = id;
        return id;
    }

    template <typename TComponent>
    ComponentTypeIndex GetTypeIndex () const {
        auto it = m_typeToId.find(typeid(TComponent));
        assert(it != m_typeToId.cend());
        return it->second;
    }

    bool GetTypeIndex (const char* name, ComponentTypeIndex& id) const {
        for (ComponentTypeIndex i = 0; i < Count(); ++i) {
            if (strcmp(m_componentTypes[i].m_name, name) == 0) {
                id = i;
                return true;
            }
        }
        return false;
    }

    const ComponentType& GetType (ComponentTypeIndex id) const { return m_componentTypes[id]; }

    ComponentTypeIndex Count () const { return static_cast<ComponentTypeIndex>(m_typeToId.size()); }
    const ComponentType& operator[] (ComponentTypeIndex index) const { return m_componentTypes[index]; }

private:
    std::array<ComponentType, ENCOSYS_MAX_COMPONENTS_> m_componentTypes;
    std::map<std::type_index, ComponentTypeIndex> m_typeToId{};
};

} // namespace ecs