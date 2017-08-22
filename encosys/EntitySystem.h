#pragma once

#include "EncosysDefines.h"
#include "EntityId.h"
#include "FunctionTraits.h"
#include "System.h"
#include <array>
#include <cassert>
#include <unordered_map>
#include <vector>

namespace ecs {

// Forward declarations
class ComponentTypeRegistry;
class Entity;

class EntityStorage {
public:
    explicit EntityStorage (EntityId id) : m_id{id} {}

    EntityId GetId                () const { return m_id; }

    bool     HasComponent         (ComponentTypeId typeId) const { return m_bitset[typeId]; }
    bool     HasComponentBitset   (ComponentBitset bitset) const { return (m_bitset & bitset) == bitset; }

    uint32_t GetComponentIndex    (ComponentTypeId typeId) const { return m_components[typeId]; }
    void     SetComponentIndex    (ComponentTypeId typeId, uint32_t index) { m_bitset.set(typeId); m_components[typeId] = index; }
    void     RemoveComponentIndex (ComponentTypeId typeId) { m_bitset.set(typeId, false); m_components[typeId] = c_invalidIndex; }

private:
    EntityId m_id;
    ComponentBitset m_bitset;
    std::array<uint32_t, ENCOSYS_MAX_COMPONENTS_> m_components;
};

class EntitySystem {
public:
    explicit EntitySystem (ComponentTypeRegistry& componentRegistry);
    virtual ~EntitySystem ();

    void                                                           Update          ();

    // Entity members
    EntityId                                                       Create          (bool active = true);
    void                                                           Destroy         (EntityId e);
    bool                                                           IsValid         (EntityId e) const;
    bool                                                           IsActive        (EntityId e) const;
    void                                                           SetActive       (EntityId e, bool active);
    uint32_t                                                       EntityCount     () const;

    // Component members
    template <typename TComponent> std::decay_t<TComponent>&       AddComponent    (EntityId e, const TComponent& component = {});
    template <typename TComponent> void                            RemoveComponent (EntityId e);
    template <typename TComponent> std::decay_t<TComponent>*       GetComponent    (EntityId e);
    template <typename TComponent> const std::decay_t<TComponent>* GetComponent    (EntityId e) const;

    // System members
    template <typename TSystem, typename... TArgs> void            RegisterSystem  (TArgs&&... args);
    template <typename TCallback> void                             ForEach         (TCallback&& callback);

private:
    // Helper members
    void IndexSwapEntities (uint32_t lhsIndex, uint32_t rhsIndex);
    bool IndexIsActive (uint32_t index) const;
    void IndexSetActive (uint32_t& index, bool active);

    template <typename TCallback, typename... Args, std::size_t... Seq>
    void UnpackAndCallback (const EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>);

    // Member variables
    ComponentTypeRegistry& m_componentRegistry;
    std::vector<System*> m_systems;
    std::unordered_map<EntityId, uint32_t> m_idToEntity;
    std::vector<EntityStorage> m_entities;
    uint32_t m_entityIdCounter{};
    uint32_t m_entityActiveCount{};
};

class Entity {
public:
    Entity (EntitySystem& entitySystem, const EntityStorage& entityStorage) :
        m_entitySystem{entitySystem},
        m_entityStorage{entityStorage} {}

    EntityId GetId () const {
        return m_entityStorage.GetId();
    }

    template <typename TComponent>
    std::decay_t<TComponent>* GetComponent () {
        return m_entitySystem.GetComponent<TComponent>(m_entityStorage.GetId());
    }

    template <typename TComponent>
    const std::decay_t<TComponent>* GetComponent () const {
        return m_entitySystem.GetComponent<TComponent>(m_entityStorage.GetId());
    }

private:
    EntitySystem& m_entitySystem;
    const EntityStorage& m_entityStorage;
};

template <typename TComponent>
std::decay_t<TComponent>& EntitySystem::AddComponent (EntityId e, const TComponent& component) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();
    const ComponentType& type = m_componentRegistry.GetType(typeId);

    // Retrieve the storage for this component type
    auto& storage = m_componentRegistry.GetStorage<TComponent>();

    // Create the component and set the component index for this entity
    uint32_t componentIndex = storage.Create(component);
    m_entities[entityIter->second].SetComponentIndex(typeId, componentIndex);
    return storage.GetObject(componentIndex);
}

template <typename TComponent>
std::decay_t<TComponent>* EntitySystem::GetComponent (EntityId e) {
    return const_cast<std::decay_t<TComponent>*>(static_cast<const EntitySystem*>(this)->GetComponent<TComponent>(e));
}

template <typename TComponent>
const std::decay_t<TComponent>* EntitySystem::GetComponent (EntityId e) const {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();
    const ComponentType& type = m_componentRegistry.GetType(typeId);

    // Retrieve the storage for this component type
    auto& storage = m_componentRegistry.GetStorage<TComponent>();

    // Find the component index for this entity and return the component
    const EntityStorage& entity = m_entities[entityIter->second];
    if (!entity.HasComponent(typeId)) {
        return nullptr;
    }
    return &storage.GetObject(entity.GetComponentIndex(typeId));
}

template <typename TComponent>
void EntitySystem::RemoveComponent (EntityId e) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();
    const ComponentType& type = m_componentRegistry.GetType(typeId);

    // Retrieve the storage for this component type
    auto& storage = m_componentRegistry.GetStorage<TComponent>();

    // Find the component index for this entity and destroy the component
    EntityStorage& entity = m_entities[entityIter->second];
    if (entity.HasComponent(typeId)) {
        storage.Destroy(entity.GetComponentIndex(typeId));
        entity.RemoveComponentIndex(typeId);
    }
}

template <typename TSystem, typename... TArgs>
void EntitySystem::RegisterSystem (TArgs&&... args) {
    static_assert(std::is_base_of<System, TSystem>::value, "TSystem must be derived from System");
    System* system = new TSystem(std::forward<TArgs>(args)...);
    assert(system != nullptr);
    m_systems.push_back(system);
}

template <typename TCallback>
void EntitySystem::ForEach (TCallback&& callback) {
    using FTraits = FunctionTraits<decltype(callback)>;
    static_assert(FTraits::ArgCount > 0, "First callback param must be ecs::Entity.");
    static_assert(std::is_same<FTraits::Arg<0>, Entity>::value, "First callback param must be ecs::Entity.");
    using FComponentArgs = typename FTraits::Args::RemoveFirst;

    ComponentBitset targetMask{};
    FComponentArgs::ForTypes([this, &targetMask] (auto t) {
        (void)t;
        assert(m_componentRegistry.HasType<TYPE_OF(t)>());
        targetMask.set(m_componentRegistry.GetTypeId<TYPE_OF(t)>());
    });

    for (uint32_t i = 0; i < m_entityActiveCount; ++i) {
        const EntityStorage& entity = m_entities[i];
        if (entity.HasComponentBitset(targetMask)) {
            UnpackAndCallback(
                entity,
                callback,
                FComponentArgs{},
                typename GenerateSequence<FTraits::ArgCount>::Type{}
            );
        }
    }
}

template <typename TCallback, typename... Args, std::size_t... Seq>
void EntitySystem::UnpackAndCallback (const EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>) {
    auto params = std::make_tuple(
        ecs::Entity(*this, entity),
        std::ref(m_componentRegistry.GetStorage<Args>().GetObject(entity.GetComponentIndex(m_componentRegistry.GetTypeId<Args>())))...
    );
    callback(std::get<Seq>(params)...);
}

} // namespace ecs
