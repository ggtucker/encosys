#pragma once

#include <vector>
#include "Encosys.h"
#include "SystemType.h"

namespace ecs {

class System {
public:
    virtual ~System () = default;

    virtual void Initialize (Encosys& encosys, SystemType& type) = 0;
    virtual void Update (SystemContext& context, TimeDelta delta) = 0;
    // virtual void UpdateParallel (Entity* entity, TimeDelta delta) = 0;
};

}