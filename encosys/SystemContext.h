#pragma once

#include <vector>
#include "ComponentDependency.h"
#include "Encosys.h"
#include "SystemType.h"

namespace ecs {

template <typename... TComponentDependencies>
class SystemContext {
private:
    using TComponentDependencyList = tmp::TypeList<TComponentDependencies...>;

    enum QueuedMod : uint8_t {
        QueuedModCreateEntity = 0,
        QueuedModAddComponent,
        QueuedModRemoveComponent
    };

    struct EntityContext {
        explicit EntityContext (Entity* storage) : m_storage{storage} {}

        Entity* m_storage{nullptr};
        std::vector<uint8_t> m_queuedModBytes{};
    };

public:
    SystemContext (Encosys& encosys, const SystemType& systemType) :
        m_encosys{encosys},
        m_systemType{&systemType} {
        m_entities.reserve(encosys.ActiveEntityCount());
        for (uint32_t e = 0; e < encosys.ActiveEntityCount(); ++e) {
            Entity& entity = encosys[e];
            if (entity.HasComponentBitset(systemType.GetRequiredBitset())) {
                m_entities.push_back(EntityContext(&entity));
            }
        }
    }

    uint32_t EntityCount () const { return static_cast<uint32_t>(m_entities.size()); }

    template <typename TComponent>
    TComponent* WriteComponent (uint32_t entityIndex) {
        using TFilteredList = typename TComponentDependencyList::Filter<typename tmp::ComponentHasWriteAccess<TComponent>::Filter>;
        static_assert(TFilteredList::Size > 0, "The system was not registered to WRITE to components of this type.");
        return m_entities[entityIndex].m_storage->GetComponent<TComponent>(m_encosys);
    }

    template <typename TComponent>
    const TComponent* ReadComponent (uint32_t entityIndex) const {
        using TFilteredList = typename TComponentDependencyList::Filter<typename tmp::ComponentHasReadAccess<TComponent>::Filter>;
        static_assert(TFilteredList::Size > 0, "The system was not registered to READ from components of this type.");
        return m_entities[entityIndex].m_storage->GetComponent<TComponent>(m_encosys);
    }

private:
    Encosys& m_encosys;
    const SystemType* m_systemType{};
    std::vector<EntityContext> m_entities{};
};

}