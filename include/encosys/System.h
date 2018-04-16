#pragma once

#include "Encosys.h"
#include "SystemIter.h"
#include "SystemType.h"

namespace ecs {

class SystemRegistry;

class System {
public:
    virtual ~System () = default;

    virtual void Initialize (SystemType& type) = 0;
    virtual void Update (TimeDelta delta) = 0;

protected:
    template <typename TComponent> void RequiredComponent (SystemType& type, Access access) {
        type.RequiredComponent(m_encosys->GetComponentTypeId<TComponent>(), access);
    }

    template <typename TComponent> void OptionalComponent (SystemType& type, Access access) {
        type.OptionalComponent(m_encosys->GetComponentTypeId<TComponent>(), access);
    }

    template <typename TSingleton> void RequiredSingleton (SystemType& type, Access access) {
        type.RequiredSingleton(m_encosys->GetSingletonTypeId<TSingleton>(), access);
    }

    SystemIter SystemIterator () { return SystemIter(*m_encosys, *m_type); }

    SystemEntity GetEntity (ecs::EntityId id) { return SystemEntity(*m_type, m_encosys->Get(id)); }

    template <typename TSingleton>
    TSingleton& WriteSingleton () {
        ENCOSYS_ASSERT_(m_type->IsSingletonWriteAllowed(m_encosys->GetSingletonTypeId<TSingleton>()));
        return m_encosys->GetSingleton<TSingleton>();
    }

    template <typename TSingleton>
    const TSingleton& ReadSingleton () const {
        ENCOSYS_ASSERT_(m_type->IsSingletonReadAllowed(m_encosys->GetSingletonTypeId<TSingleton>()));
        return m_encosys->GetSingleton<TSingleton>();
    }

private:
    friend class SystemRegistry;
    Encosys* m_encosys;
    SystemType* m_type;
};

}