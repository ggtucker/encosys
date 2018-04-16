#pragma once

#include "BlockObjectPool.h"
#include "EncosysConfig.h"
#include "SystemType.h"
#include <array>
#include <map>
#include <typeindex>

namespace ecs {

class Encosys;
class System;

class SystemRegistry {
public:
    virtual ~SystemRegistry ();

    template <typename TSystem>
    SystemTypeId Register (Encosys& encosys) {
        static_assert(std::is_base_of<System, TSystem>::value, "TSystem must be derived from System");
        using TDecayed = std::decay_t<TSystem>;
        const SystemTypeId id = Count();
        ENCOSYS_ASSERT_(m_typeToId.find(typeid(TDecayed)) == m_typeToId.end());
        SystemType& systemType = m_systemTypes[id];
        systemType = SystemType(id);
        m_typeToId[typeid(TDecayed)] = id;

        System* system = new TSystem();
        system->m_encosys = &encosys;
        system->m_type = &systemType;
        ENCOSYS_ASSERT_(system != nullptr);
        m_systems.push_back(system);
        return id;
    }

    template <typename TSystem>
    SystemTypeId GetTypeId () const {
        auto it = m_typeToId.find(typeid(std::decay_t<TSystem>));
        ENCOSYS_ASSERT_(it != m_typeToId.cend());
        return it->second;
    }

    const SystemType& GetType (SystemTypeId id) const {
        ENCOSYS_ASSERT_(id < Count());
        return m_systemTypes[id];
    }

    bool HasType (SystemTypeId id) const { return id < Count(); }

    template <typename TSystem>
    bool HasType () { return HasType(GetTypeId<TSystem>()); }

    uint32_t Count () const { return static_cast<uint32_t>(m_typeToId.size()); }

    System* GetSystem (uint32_t index) { return m_systems[index]; }
    const System* GetSystem (uint32_t index) const { return m_systems[index]; }

    SystemType& GetSystemType (uint32_t index) { return m_systemTypes[index]; }
    const SystemType& GetSystemType (uint32_t index) const { return m_systemTypes[index]; }

private:
    std::vector<System*> m_systems{};
    std::array<SystemType, ENCOSYS_MAX_COMPONENTS_> m_systemTypes;
    std::map<std::type_index, SystemTypeId> m_typeToId{};
};

} // namespace ecs