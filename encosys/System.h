#pragma once

#include <vector>
#include "ComponentDependency.h"
#include "Encosys.h"
#include "SystemContext.h"
#include "TypeUtil.h"

namespace ecs {

class System {
public:
    virtual ~System() = default;

    virtual void Initialize (Encosys& encosys) = 0;
    virtual void Update (Encosys& encosys, const SystemType& type, TimeDelta delta) = 0;
    // virtual void UpdateParallel (Entity* entity, TimeDelta delta) = 0;
};

template <typename... TComponentDependencies>
class SequentialSystem : public System {
public:
    static_assert(
        tmp::AllEqualTo<bool, true, tmp::IsComponentDependency<TComponentDependencies>::value...>,
        "The template parameters for SequentialSystem must all be of the ecs::ComponentDependency type."
    );
    using SystemContext = SystemContext<TComponentDependencies...>;

    virtual void Update (SystemContext& context, TimeDelta delta) = 0;

private:
    virtual void Update (Encosys& encosys, const SystemType& type, TimeDelta delta) override {
        SystemContext context(encosys, type);
        Update(context, delta);
    }
};

template <typename... TComponentDependencies>
class ParallelSystem : public System {
    static_assert(
        tmp::AllEqualTo<bool, true, tmp::IsComponentDependency<TComponentDependencies>::value...>,
        "The template parameters for ParallelSystem must all be of the ecs::ComponentDependency type."
    );
public:
};

}