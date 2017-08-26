#pragma once

#include "BlockObjectPool.h"
#include "EncosysDefines.h"
#include "SystemType.h"
#include <array>
#include <cassert>
#include <map>
#include <typeindex>

namespace ecs {

class System;

class SystemRegistry {
public:
    virtual ~SystemRegistry ();

    template <typename TSystem, typename... TArgs>
    SystemTypeId Register (TArgs&&... args) {
        static_assert(std::is_base_of<System, TSystem>::value, "TSystem must be derived from System");
        using TDecayed = std::decay_t<TSystem>;
        const SystemTypeId id = Count();
        assert(m_typeToId.find(typeid(TDecayed)) == m_typeToId.end());
        m_systemTypes[id] = SystemType(id);
        m_typeToId[typeid(TDecayed)] = id;

        System* system = new TSystem(std::forward<TArgs>(args)...);
        assert(system != nullptr);
        m_systems.push_back(system);
        return id;
    }

    template <typename TSystem>
    SystemTypeId GetTypeId () const {
        auto it = m_typeToId.find(typeid(std::decay_t<TSystem>));
        assert(it != m_typeToId.cend());
        return it->second;
    }

    const SystemType& GetType (SystemTypeId id) const {
        assert(id < Count());
        return m_systemTypes[id];
    }

    bool HasType (SystemTypeId id) const {
        return id < Count();
    }

    template <typename TSystem>
    bool HasType () {
        return HasType(GetTypeId<TSystem>());
    }

    System* GetSystem (uint32_t index) { return m_systems[index]; }
    const System* GetSystem (uint32_t index) const { return m_systems[index]; }

    uint32_t Count () const { return static_cast<uint32_t>(m_typeToId.size()); }
    SystemType& operator[] (uint32_t index) { return m_systemTypes[index]; }
    const SystemType& operator[] (uint32_t index) const { return m_systemTypes[index]; }

private:
    std::vector<System*> m_systems{};
    std::array<SystemType, ENCOSYS_MAX_COMPONENTS_> m_systemTypes;
    std::map<std::type_index, SystemTypeId> m_typeToId{};
};

} // namespace ecs