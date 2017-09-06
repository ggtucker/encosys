# encosys
**encosys** is an **en**tity **co**mponent **sys**tem (ECS) framework being developed by Geoffrey Tucker. ECS decouples code from data, and data from objects. There are many benefits that arise from an approach like this, including but not limited to:
* **Maintainability**: dependencies between systems are explicitly defined, yet abstracted from the systems themselves.
* **Flexibility**: entities are defined only by their components, and components can be added and removed at runtime.
* **Performance**: systems are easily parallelized, and components are stored with cache coherency in mind.

## components
A component is a plain old data structure that can be attached to an entity. 

```cpp
struct Position {
    Position () {}
    Position (float x, float y) : x{x}, y{y} {}
    float x;
    float y;
};
```

## entities
An entity is just an ID. From this ID we can add, remove, and query for components.
```cpp
ecs::Encosys encosys;
ecs::EntityId entityId = encosys.Create();
```
#### adding a component
The component does not need to have a default constructor, but if there isn't one then the constructor's parameters must be passed in when adding the component to an entity.
```cpp
// Using default constructor
encosys.AddComponent<Position>(entityId);

// Using non-default constructor
encosys.AddComponent<Position>(entityId, 5.f, 10.f);
```
#### getting and removing a component
```cpp
Position* position = encosys.GetComponent<Position>(entityId);
if (position) {
    // Do stuff with position if the entity had this component.
}
encosys.RemoveComponent<Position>(entityId);
```

## systems
A system runs logic on the entities that have a specific subset of components. Systems must inherit from either ecs::SequentialSystem or ecs::ParallelSystem and implement the Initialize and Update functions.

Since systems register their component dependencies using the variadic template interface on their base class, they have the added benefit of compile-time verification of proper component usage. In the example below, the code would fail at compile time if a user tried to write to the Acceleration component (which was only registered for Read access).
```cpp
struct PhysicsSystem : public ecs::ParallelSystem<
    ecs::ComponentDependency<Position, ecs::Existence::Required, ecs::Access::Write>,
    ecs::ComponentDependency<Velocity, ecs::Existence::Required, ecs::Access::Write>,
    ecs::ComponentDependency<Acceleration, ecs::Existence::Optional, ecs::Access::Read>
> {
    virtual void Initialize (ecs::Encosys& encosys) override {
        // Do initialization things.
    }
    
    virtual void Update (SystemContext& context, ecs::TimeDelta delta) override {
        // Update this system given a time delta.
        for (uint32_t i = 0; i < context.EntityCount(); ++i) {
            Velocity& velocity = *context.WriteComponent<Velocity>(i);
            // Since acceleration was flagged as an optional component, we must check its existence.
            if (const CAcceleration* acceleration = context.ReadComponent<CAcceleration>(i)) {
                velocity.x += acceleration->x * delta;
                velocity.y += acceleration->y * delta;
            }
            Position& position = *context.WriteComponent<Position>(i);
            position.x += velocity.x * delta;
            position.y += velocity.y * delta;
        }
    }
};
```

## iterating entities outside systems
Entities can be iterated using a lambda or for loop, but it is generally discouraged since only ecs::System can benefit from parallelization.
```cpp
// Iterate through every entity that has a Position and Velocity component

// Using a lambda
encosys.ForEach([&encosys, delta](ecs::Entity& entity, Position& position, Velocity& velocity) {
    // Since acceleration is not required, we must check its existence.
    if (const Acceleration* acceleration = entity.GetComponent<Acceleration>(encosys)) {
        velocity.x += acceleration->x * delta;
        velocity.y += acceleration->y * delta;
    }
    position.x += velocity.x * delta;
    position.y += velocity.y * delta;
});

// Using a for loop
const ecs::ComponentBitset requiredBitset = (
    encosys.GetComponentType<Position>().Id()
    | encosys.GetComponentType<Velocity>().Id()
);
for (uint32_t e = 0; e < encosys.ActiveEntityCount(); ++e) {
    ecs::Entity& entity = encosys[e];
    if (!entity.HasComponentBitset(requiredBitset)) {
        continue;
    }
    const Position& position = *entity.GetComponent<Position>(encosys);
    const Velocity& velocity = *entity.GetComponent<Velocity>(encosys);
    // Since acceleration is not required, we must check its existence.
    if (const Acceleration* acceleration = entity.GetComponent<Acceleration>(encosys)) {
        velocity.x += acceleration->x * delta;
        velocity.y += acceleration->y * delta;
    }
    position.x += velocity.x * delta;
    position.y += velocity.y * delta;
}
```

## setting up encosys
```cpp
// 1. create the framework wrapper
ecs::Encosys encosys;

// 2. register component types
encosys.RegisterComponent<Position>();
encosys.RegisterComponent<Velocity>();
encosys.RegisterComponent<Acceleration>();

// 3. register systems
encosys.RegisterSystem<PhysicsSystem>();

// 4. create entities
ecs::EntityId entityId = encosys.Create();

// 5. add components to entities
encosys.AddComponent<Position>(entityId, 5.f, 10.f);

// 6. initialize systems
encosys.Initialize();

// 7. run game loop
while (running) {
    encosys.Update(0.0625f);
}
```
