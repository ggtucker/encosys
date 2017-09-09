#pragma once

#include <vector>
#include "ComponentDependency.h"
#include "Encosys.h"
#include "SystemType.h"

namespace ecs {

template <typename TComponentDependencyList>
class SystemEntity {
public:
    explicit SystemEntity (Entity entity) :
        m_entity{entity} {
    }

    template <typename TComponent>
    TComponent* WriteComponent () {
        using TFilteredList = typename TComponentDependencyList::Filter<typename tmp::ComponentHasWriteAccess<TComponent>::Filter>;
        static_assert(TFilteredList::Size > 0, "The system was not registered to WRITE to components of this type.");
        return m_entity.GetComponent<TComponent>();
    }

    template <typename TComponent>
    const TComponent* ReadComponent () const {
        using TFilteredList = typename TComponentDependencyList::Filter<typename tmp::ComponentHasReadAccess<TComponent>::Filter>;
        static_assert(TFilteredList::Size > 0, "The system was not registered to READ from components of this type.");
        return m_entity.GetComponent<TComponent>();
    }

private:
    Entity m_entity;
};

template <typename TComponentDependencyList>
class SystemContext {
public:
    using SystemEntity = SystemEntity<TComponentDependencyList>;

    explicit SystemContext (Encosys& encosys, const SystemType& systemType) {
        m_entities.reserve(encosys.ActiveEntityCount());
        for (uint32_t e = 0; e < encosys.ActiveEntityCount(); ++e) {
            Entity entity = encosys[e];
            if (entity.HasComponentBitset(systemType.GetRequiredBitset())) {
                m_entities.push_back(SystemEntity(entity));
            }
        }
    }

    uint32_t EntityCount () const { return static_cast<uint32_t>(m_entities.size()); }
    SystemEntity GetEntity (uint32_t index) { return SystemEntity(m_entities[index]); }

private:
    std::vector<SystemEntity> m_entities{};
};

}