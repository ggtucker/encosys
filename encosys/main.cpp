#include "ComponentTypeRegistry.h"
#include "EntitySystem.h"
#include <iostream>

struct CPosition {
    float x;
    float y;
};

void OutputPosition (const CPosition& position) {
    std::cout << "x: " << position.x << " y: " << position.y << std::endl;
}

int main () {
    uint32_t componentIndex = 0;

    ecs::ComponentTypeRegistry componentRegistry;
    componentRegistry.Register<CPosition>("Position");

    ecs::EntitySystem entitySystem(componentRegistry);

    ecs::Entity e = entitySystem.Create();
    CPosition& position = entitySystem.Add<CPosition>(e);
    position.x = 5;
    position.y = 10;

    CPosition& samePosition = entitySystem.Get<CPosition>(e);
    OutputPosition(position);
    OutputPosition(samePosition);

    return 0;
}