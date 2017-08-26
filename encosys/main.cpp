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

    virtual void Initialize (ecs::EntitySystem& entitySystem, ecs::SystemType& type) override {
        type.UseComponent<CPosition>(entitySystem, ecs::ComponentRequirement::Required, ecs::ComponentUsage::ReadOnly);
    }

    virtual void Update (ecs::EntitySystem&, const std::vector<ecs::Entity>& entities, ecs::TimeDelta delta) override {
        for (const ecs::Entity& entity : entities) {
            assert(entity.ReadComponent<CPosition>() != nullptr);
            OutputPosition("System", entity.GetId(), entity.ReadComponent<CPosition>());
        }
    }

};

int main () {
    uint32_t componentIndex = 0;

    ecs::EntitySystem encosys;

    // 1. register component types
    encosys.RegisterComponent<CPosition>();

    // 2. register systems
    encosys.RegisterSystem<SPhysics>();

    // 3. create entities
    ecs::EntityId e = encosys.Create();

    // 4. add components to entities
    CPosition pos;
    pos.x = 5;
    pos.y = 10;
    encosys.AddComponent<CPosition>(e) = pos;

    CPosition* samePosition = encosys.GetComponent<CPosition>(e);
    samePosition->y = 7;
    
    encosys.ForEach([](ecs::Entity entity, CPosition& pos) {
        OutputPosition("ForEach1", entity.GetId(), &pos);
        OutputPosition("ForEach2", entity.GetId(), entity.ReadComponent<CPosition>());
    });

    encosys.Initialize();
    encosys.Update(0.0625f);

    return 0;
}