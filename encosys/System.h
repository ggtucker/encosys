#pragma once

#include <vector>
#include "ComponentDependency.h"
#include "ComponentRegistry.h"
#include "Encosys.h"
#include "SystemContext.h"
#include "TypeUtil.h"

namespace ecs {

class System {
public:
    virtual ~System() = default;

    virtual void Update (Encosys& encosys, const SystemType& systemType, TimeDelta delta) = 0;

    virtual ComponentBitset GetRequiredComponents (const ComponentRegistry& componentRegistry) const = 0;
};

template <typename... TComponentDependencies>
class SequentialSystem : public System {
public:
    static_assert(
        tmp::AllEqualTo<bool, true, tmp::IsComponentDependency<TComponentDependencies>::value...>,
        "The template parameters for SequentialSystem must all be of the ecs::ComponentDependency type."
    );
    using ComponentDependencyList = tmp::TypeList<TComponentDependencies...>;
    using SystemContext = SystemContext<ComponentDependencyList>;
    using SystemEntity = typename SystemContext::SystemEntity;

    virtual void Update (SystemContext& context, TimeDelta delta) = 0;

private:
    virtual void Update (Encosys& encosys, const SystemType& systemType, TimeDelta delta) override {
        SystemContext context(encosys, systemType);
        Update(context, delta);
    }

    virtual ComponentBitset GetRequiredComponents (const ComponentRegistry& componentRegistry) const override {
        ComponentBitset bitset;
        ComponentDependencyList::ForTypes([&bitset, &componentRegistry](auto t) {
            (void)t;
            using DependencyType = TYPE_OF(t);
            if (DependencyType::Existence == Existence::Required) {
                bitset.set(componentRegistry.GetTypeId<typename DependencyType::Component>());
            }
        });
        return bitset;
    }
};

template <typename... TComponentDependencies>
class ParallelSystem : public System {
public:
    static_assert(
        tmp::AllEqualTo<bool, true, tmp::IsComponentDependency<TComponentDependencies>::value...>,
        "The template parameters for ParallelSystem must all be of the ecs::ComponentDependency type."
    );
    using ComponentDependencyList = tmp::TypeList<TComponentDependencies...>;
    using SystemEntity = SystemEntity<ComponentDependencyList>;

    virtual void Update (SystemEntity entity, TimeDelta delta) = 0;

private:
    virtual void Update (Encosys& encosys, const SystemType& systemType, TimeDelta delta) override {
        for (uint32_t e = 0; e < encosys.ActiveEntityCount(); ++e) {
            Entity& entity = encosys[e];
            if (entity.HasComponentBitset(systemType.GetRequiredBitset())) {
                Update(SystemEntity(encosys, systemType, entity), delta);
            }
        }
    }

    virtual ComponentBitset GetRequiredComponents (const ComponentRegistry& componentRegistry) const override {
        ComponentBitset bitset;
        ComponentDependencyList::ForTypes([&bitset, &componentRegistry](auto t) {
            (void)t;
            using DependencyType = TYPE_OF(t);
            if (DependencyType::Existence == Existence::Required) {
                bitset.set(componentRegistry.GetTypeId<typename DependencyType::Component>());
            }
        });
        return bitset;
    }
};

}