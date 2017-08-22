#pragma once

#include <vector>
#include "EncosysDefines.h"

namespace ecs {

// Forward declarations
class Entity;

class System {
public:

    virtual ~System () = default;

    void RequireComponent (ecs::ComponentTypeId componentId) { m_requiredComponents.set(componentId); }
    ComponentBitset GetRequiredComponents () const { return m_requiredComponents; }

    virtual void Update (const std::vector<Entity>& entities) = 0;

private:
    ComponentBitset m_requiredComponents;
};

}