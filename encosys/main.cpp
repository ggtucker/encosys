#include "ComponentTypeRegistry.h"
#include "EntitySystem.h"
#include "System.h"
#include <iostream>

struct CPosition {
    float x;
    float y;
};

void OutputPosition (const char* prefix, ecs::EntityId id, const CPosition* position) {
    if (position) {
        std::cout << "[" << prefix << "] (id=" << id.Id() << ") x: " << position->x << " y: " << position->y << std::endl;
    }
}

struct SPhysics : public ecs::System {

    virtual void Update (const std::vector<ecs::Entity>& entities) override {
        for (const ecs::Entity& entity : entities) {
            OutputPosition("System", entity.GetId(), entity.GetComponent<CPosition>());
        }
    }

};

int main () {
    uint32_t componentIndex = 0;

    ecs::ComponentTypeRegistry componentRegistry;
    ecs::ComponentTypeId positionId = componentRegistry.Register<CPosition>("Position");

    SPhysics physicsSystem;
    physicsSystem.RequireComponent(positionId);

    ecs::EntitySystem encosys(componentRegistry);
    encosys.RegisterSystem<SPhysics>(physicsSystem);

    ecs::EntityId e = encosys.Create();

    CPosition pos;
    pos.x = 5;
    pos.y = 10;

    encosys.AddComponent<CPosition>(e) = pos;

    CPosition* samePosition = encosys.GetComponent<CPosition>(e);
    samePosition->y = 7;
    
    encosys.ForEach([](ecs::Entity entity, CPosition& pos) {
        OutputPosition("ForEach1", entity.GetId(), &pos);
        OutputPosition("ForEach2", entity.GetId(), entity.GetComponent<CPosition>());
    });

    encosys.Update();

    return 0;
}