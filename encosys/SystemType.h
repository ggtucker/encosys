#pragma once

#include "EncosysDefines.h"

namespace ecs {

enum class ComponentUsage {
    Read,
    Write
};

class SystemType {
public:
    SystemType () {}
    explicit SystemType (SystemTypeId id) : m_id{id} {}

    SystemTypeId Id () const { return m_id; }

    void UseAllComponents () {
        m_readBitset.set();
        m_writeBitset.set();
    }

    void RequireComponent (ComponentTypeId type, ComponentUsage usage) {
        m_requiredBitset.set(type, true);
        OptionalizeComponent(type, usage);
    }

    void OptionalizeComponent (ComponentTypeId type, ComponentUsage usage) {
        m_readBitset.set(type);
        m_writeBitset.set(type, usage == ComponentUsage::Write);
    }

    const ComponentBitset& GetRequiredBitset () const { return m_requiredBitset; }

    bool IsReadAllowed (ComponentTypeId typeId) const { return m_readBitset.test(typeId); }
    bool IsWriteAllowed (ComponentTypeId typeId) const { return m_writeBitset.test(typeId); }

private:
    SystemTypeId m_id{};
    ComponentBitset m_requiredBitset{};
    ComponentBitset m_readBitset{};
    ComponentBitset m_writeBitset{};
};

}