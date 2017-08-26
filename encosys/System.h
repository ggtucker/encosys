#pragma once

#include <vector>
#include "Encosys.h"
#include "SystemType.h"

namespace ecs {

class System {
public:
    virtual ~System () = default;

    virtual void Initialize (Encosys& encosys, SystemType& type) = 0;
    virtual void Update (Encosys& encosys, const std::vector<Entity>& entities, TimeDelta delta) = 0;
};

}