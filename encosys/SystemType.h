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

    void RequireComponent (ComponentTypeId type, ComponentUsage usage) {
        m_requiredBitset.set(type, true);
    }

    const ComponentBitset& GetRequiredBitset () const { return m_requiredBitset; }

private:
    SystemTypeId m_id{};
    ComponentBitset m_requiredBitset{};
};

}