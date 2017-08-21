#include "ComponentTypeRegistry.h"
#include "EntitySystem.h"
#include <iostream>

struct CPosition {
    float x;
    float y;
};

void OutputPosition (const CPosition* position) {
    if (position) {
        std::cout << "x: " << position->x << " y: " << position->y << std::endl;
    }
}

int main () {
    uint32_t componentIndex = 0;

    ecs::ComponentTypeRegistry componentRegistry;
    componentRegistry.Register<CPosition>("Position");

    ecs::EntitySystem entitySystem(componentRegistry);

    ecs::EntityId e = entitySystem.Create();

    CPosition pos;
    pos.x = 5;
    pos.y = 10;

    entitySystem.Add<CPosition>(e) = pos;

    CPosition* samePosition = entitySystem.Get<CPosition>(e);
    samePosition->y = 7;
    
    entitySystem.ForEach([](ecs::Entity entity, CPosition& pos) {
        OutputPosition(&pos);
    });

    return 0;
}