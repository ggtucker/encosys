#pragma once

#include "EncosysConfig.h"
#include "VirtualObject.h"
#include <array>
#include <cassert>
#include <map>
#include <typeindex>

namespace ecs {

class SingletonRegistry {
public:
    virtual ~SingletonRegistry () {
        for (uint32_t i = 0; i < Count(); ++i) {
            m_singletons[i].Clear();
        }
    }

    template <typename TSingleton>
    SingletonTypeId Register () {
        using TDecayed = std::decay_t<TSingleton>;
        const SingletonTypeId id = Count();
        assert(id < ENCOSYS_MAX_SINGLETONS_);
        assert(m_typeToId.find(typeid(TDecayed)) == m_typeToId.end());
        m_typeToId[typeid(TDecayed)] = id;
        m_singletons[id] = VirtualObject(TDecayed());
        return id;
    }

    template <typename TSingleton>
    SingletonTypeId GetTypeId () const {
        auto it = m_typeToId.find(typeid(std::decay_t<TSingleton>));
        assert(it != m_typeToId.cend());
        return it->second;
    }

    bool HasType (SingletonTypeId id) const {
        return id < Count();
    }

    template <typename TSingleton>
    bool HasType () {
        return HasType(GetTypeId<TSingleton>());
    }

    template <typename TSingleton>
    auto& GetSingleton () { return GetSingleton(GetTypeId<TSingleton>()).Get<std::decay_t<TSingleton>>(); }

    template <typename TSingleton>
    const auto& GetSingleton () const { return GetSingleton(GetTypeId<TSingleton>()).Get<const std::decay_t<TSingleton>>(); }

    VirtualObject& GetSingleton (SingletonTypeId id) { assert(id < Count()); return m_singletons[id]; }
    const VirtualObject& GetSingleton (SingletonTypeId id) const { assert(id < Count()); return m_singletons[id]; }

    uint32_t Count () const { return static_cast<uint32_t>(m_typeToId.size()); }

private:
    std::array<VirtualObject, ENCOSYS_MAX_SINGLETONS_> m_singletons{};
    std::map<std::type_index, SingletonTypeId> m_typeToId{};
};

} // namespace ecs