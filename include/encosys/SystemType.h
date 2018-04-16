#pragma once

#include "EncosysConfig.h"

namespace ecs {

enum class Access {
    Read,
    Write
};

class SystemType {
public:
    SystemType () {}
    explicit SystemType (SystemTypeId id) : m_id{id} {}

    SystemTypeId Id () const { return m_id; }

    void RequiredComponent (ComponentTypeId type, Access access) {
        m_requiredComponents.set(type, true);
        OptionalComponent(type, access);
    }

    void OptionalComponent (ComponentTypeId type, Access access) {
        m_readComponents.set(type);
        m_writeComponents.set(type, access == Access::Write);
    }

    void RequiredSingleton (SingletonTypeId type, Access access) {
        m_readSingletons.set(type);
        m_writeSingletons.set(type, access == Access::Write);
    }

    const ComponentBitset& GetRequiredBitset () const { return m_requiredComponents; }

    bool IsComponentReadAllowed (ComponentTypeId typeId) const { return m_readComponents.test(typeId); }
    bool IsComponentWriteAllowed (ComponentTypeId typeId) const { return m_writeComponents.test(typeId); }

    bool IsSingletonReadAllowed (SingletonTypeId typeId) const { return m_readSingletons.test(typeId); }
    bool IsSingletonWriteAllowed (SingletonTypeId typeId) const { return m_writeSingletons.test(typeId); }

private:
    SystemTypeId m_id{};
    ComponentBitset m_requiredComponents{};
    ComponentBitset m_readComponents{};
    ComponentBitset m_writeComponents{};
    SingletonBitset m_readSingletons{};
    SingletonBitset m_writeSingletons{};
};

}