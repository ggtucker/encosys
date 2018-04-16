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
// Create the entity management class
ecs::Encosys encosys;

// The ecs::Entity class wraps the entity id and provides a convenient interface for its components
ecs::Entity entity = encosys.Create();

// To safely store a reference to the entity, ecs::EntityId should be used instead
ecs::EntityId entityId = entity.GetId();
```
#### adding a component
The component does not need to have a default constructor, but if there isn't one then the constructor's parameters must be passed in when adding the component to an entity.
```cpp
// Adding a component using ecs::Entity
entity.AddComponent<Position>(); // default constructor
entity.AddComponent<Position>(5.f, 10.f); // non-default constructor

// Adding a component using ecs::EntityId
encosys.AddComponent<Position>(entityId); // default constructor
encosys.AddComponent<Position>(entityId, 5.f, 10.f); // non-default constructor
```
#### getting and removing a component
```cpp
// Using ecs::Entity
Position* position = entity.GetComponent<Position>();
if (position) {
    // Do stuff with position if the entity had this component.
}
entity.RemoveComponent<Position>();

// Using ecs::EntityId
Position* position = encosys.GetComponent<Position>(entityId);
if (position) {
    // Do stuff with position if the entity had this component.
}
encosys.RemoveComponent<Position>(entityId);
```

## systems
A system runs logic on the entities that have a specific subset of components. Systems must inherit from ecs::System and implement the Initialize and Update functions.

Systems are expected to register their read/write component dependencies in the Initialize function which can be used to infer which systems are safe to run concurrently. In the example below, the code would assert at runtime if a user tried to write to the Acceleration component (call `entity.WriteComponent<CAcceleration>()`) which was only registered for Read access.
```cpp
struct PhysicsSystem : public ecs::System {
    virtual void Initialize (ecs::SystemType& type) override {
        // Register component dependencies
        RequiredComponent<Position>(type, ecs::Access::Write);
        RequiredComponent<Velocity>(type, ecs::Access::Write);
        OptionalComponent<Acceleration>(type, ecs::Access::Read);
        // Do other initialization things.
    }
    
    virtual void Update (SystemEntity& entity, ecs::TimeDelta delta) override {
        // Update this entity given a time delta.
        Velocity& velocity = *entity.WriteComponent<Velocity>();
        // Since acceleration was flagged as an optional component, we must check for its existence.
        if (const CAcceleration* acceleration = entity.ReadComponent<CAcceleration>()) {
            velocity.x += acceleration->x * delta;
            velocity.y += acceleration->y * delta;
        }
        Position& position = *entity.WriteComponent<Position>();
        position.x += velocity.x * delta;
        position.y += velocity.y * delta;
    }
};
```

## iterating entities outside systems
Entities can be iterated using a lambda or for loop, but it is generally discouraged since only ecs::System benefits from concurrency.
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

// 3. register singleton types
encosys.RegisterSingleton<InputManager>();

// 4. register systems
encosys.RegisterSystem<PhysicsSystem>();

// 5. create entities
ecs::Entity entity = encosys.Create();

// 6. add components to entities
entity.AddComponent<Position>(5.f, 10.f);

// 7. initialize systems
encosys.Initialize();

// 8. run game loop
while (running) {
    encosys.Update(0.0625f);
}
```
