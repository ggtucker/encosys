#pragma once

#include "Encosys.h"
#include "SystemType.h"

namespace ecs {

class SystemEntity {
public:
    SystemEntity (const SystemType& type, Entity entity) :
        m_type{type},
        m_entity{entity} {
    }

    bool IsValid () const { return m_entity.IsValid(); }

    template <typename TComponent>
    TComponent* WriteComponent () {
        ENCOSYS_ASSERT_(m_type.IsComponentWriteAllowed(m_entity.GetComponentTypeId<TComponent>()));
        return m_entity.GetComponent<TComponent>();
    }

    template <typename TComponent>
    const TComponent* ReadComponent () const {
        ENCOSYS_ASSERT_(m_type.IsComponentReadAllowed(m_entity.GetComponentTypeId<TComponent>()));
        return m_entity.GetComponent<TComponent>();
    }

private:
    const SystemType& m_type;
    Entity m_entity;
};

class SystemIterType {
public:
    SystemIterType (Encosys& encosys, const SystemType& type, uint32_t index) :
        m_encosys{encosys},
        m_type{type},
        m_index{index} {
        Next();
    }

    bool operator== (SystemIterType rhs) { return m_index == rhs.m_index; }
    bool operator!= (SystemIterType rhs) { return m_index != rhs.m_index; }
    SystemEntity operator* () { return SystemEntity(m_type, m_encosys[m_index]); }
    void operator++ () { ++m_index; Next(); }

private:

    void Next () {
        while (m_index < m_encosys.ActiveEntityCount()) {
            Entity entity = m_encosys[m_index];
            if (entity.HasComponentBitset(m_type.GetRequiredBitset())) {
                break;
            }
            ++m_index;
        }
    }

    Encosys& m_encosys;
    const SystemType& m_type;
    uint32_t m_index;
};

class SystemIter {
public:
    explicit SystemIter (Encosys& encosys, const SystemType& type) : m_encosys{encosys}, m_type{type} {}

    SystemIterType begin () { return SystemIterType(m_encosys, m_type, 0); }
    SystemIterType end () { return SystemIterType(m_encosys, m_type, m_encosys.ActiveEntityCount()); }

private:
    Encosys& m_encosys;
    const SystemType& m_type;
};

}