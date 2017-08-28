#pragma once

#include "EncosysDefines.h"

namespace ecs {

class ComponentType {
public:
    ComponentType () {}
    ComponentType (ComponentTypeId id, uint32_t bytes) :
        m_id{ id },
        m_bytes{ bytes } {
    }

    ComponentTypeId Id () const { return m_id; }
    uint32_t Bytes () const { return m_bytes; }

private:
    ComponentTypeId m_id{};
    uint32_t m_bytes{};
};

}