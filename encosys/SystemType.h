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

    const ComponentBitset& GetRequiredBitset () const { return m_requiredBitset; }
    void SetRequiredComponents (ComponentBitset bitset) { m_requiredBitset = bitset; }

private:
    SystemTypeId m_id{};
    ComponentBitset m_requiredBitset{};
};

}