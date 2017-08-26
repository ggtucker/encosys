#pragma once

#include "EncosysDefines.h"

namespace ecs {

enum class ComponentRequirement {
    Required,
    Optional
};

enum class ComponentUsage {
    ReadOnly,
    Write
};

class SystemType {
public:
    SystemType () {}
    explicit SystemType (SystemTypeId id) : m_id{id} {}

    SystemTypeId Id () const { return m_id; }

    void UseComponent (ComponentTypeId typeId, ComponentRequirement requirement, ComponentUsage usage) {
        m_requiredComponents.set(typeId, requirement == ComponentRequirement::Required);
        m_readComponents.set(typeId);
        m_writeComponents.set(typeId, usage == ComponentUsage::Write);
    }

    const ComponentBitset& GetRequiredComponents () const { return m_requiredComponents; }

    bool IsReadAllowed (ComponentTypeId typeId) const { return m_readComponents.test(typeId); }
    bool IsWriteAllowed (ComponentTypeId typeId) const { return m_writeComponents.test(typeId); }

private:
    SystemTypeId m_id{};
    ComponentBitset m_requiredComponents{};
    ComponentBitset m_readComponents{};
    ComponentBitset m_writeComponents{};
};

}