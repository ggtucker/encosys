#pragma once

#include <vector>
#include "EncosysDefines.h"

namespace ecs {

// Forward declarations
class Entity;
class EntitySystem;
class SystemType;

class System {
public:
    virtual ~System () = default;

    virtual void Initialize (EntitySystem& entitySystem, SystemType& type) = 0;
    virtual void Update (EntitySystem& entitySystem, const std::vector<Entity>& entities) = 0;
};

}