#pragma once

#include <vector>
#include "EntitySystem.h"
#include "SystemType.h"

namespace ecs {

class System {
public:
    virtual ~System () = default;

    virtual void Initialize (EntitySystem& entitySystem, SystemType& type) = 0;
    virtual void Update (EntitySystem& entitySystem, const std::vector<Entity>& entities, TimeDelta delta) = 0;
};

}