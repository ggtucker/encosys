#pragma once

#include "ComponentRegistry.h"
#include "EncosysDefines.h"
#include "EntityId.h"
#include "FunctionTraits.h"
#include "SystemRegistry.h"
#include <array>
#include <cassert>
#include <unordered_map>
#include <vector>

namespace ecs {

class Entity {
public:
    explicit Entity        (EntityId id) : m_id{id} {}

    EntityId GetId                () const { return m_id; }

    bool     HasComponent         (ComponentTypeId typeId) const { return m_bitset[typeId]; }
    bool     HasComponentBitset   (ComponentBitset bitset) const { return (m_bitset & bitset) == bitset; }

    uint32_t GetComponentIndex    (ComponentTypeId typeId) const { return m_components[typeId]; }
    void     SetComponentIndex    (ComponentTypeId typeId, uint32_t index) { m_bitset.set(typeId); m_components[typeId] = index; }
    void     RemoveComponentIndex (ComponentTypeId typeId) { m_bitset.set(typeId, false); m_components[typeId] = c_invalidIndex; }

    template <typename TComponent> TComponent*       GetComponent (const Encosys& encosys);
    template <typename TComponent> const TComponent* GetComponent (const Encosys& encosys) const;

private:
    EntityId m_id;
    ComponentBitset m_bitset;
    std::array<uint32_t, ENCOSYS_MAX_COMPONENTS_> m_components;
};

class Encosys {
public:
    // Constructors
    Encosys () = default;

    // Entity members
    EntityId                                                      Create               (bool active = true);
    EntityId                                                      Copy                 (EntityId e, bool active = true);
    void                                                          Destroy              (EntityId e);
    bool                                                          IsValid              (EntityId e) const;
    bool                                                          IsActive             (EntityId e) const;
    void                                                          SetActive            (EntityId e, bool active);
    uint32_t                                                      EntityCount          () const;
    uint32_t                                                      ActiveEntityCount    () const;

    // Component members
    template <typename TComponent> ComponentTypeId                RegisterComponent    ();
    template <typename TComponent, typename... TArgs> TComponent& AddComponent         (EntityId e, TArgs&&... args);
    template <typename TComponent> void                           RemoveComponent      (EntityId e);
    template <typename TComponent> TComponent*                    GetComponent         (EntityId e);
    template <typename TComponent> const TComponent*              GetComponent         (EntityId e) const;
    template <typename TComponent> const ComponentType&           GetComponentType     () const;

    // System members
    template <typename TSystem, typename... TArgs>   void         RegisterSystem       (TArgs&&... args);
    void                                                          Initialize           ();
    void                                                          Update               (TimeDelta delta);

    // Iteration members
    template <typename TCallback> void                            ForEach              (TCallback&& callback);
    Entity&                                                       operator[]           (uint32_t index) { return m_entities[index]; }
    const Entity&                                                 operator[]           (uint32_t index) const { return m_entities[index]; }

private:
    friend class Entity;

    // Helper members
    void IndexSwapEntities (uint32_t lhsIndex, uint32_t rhsIndex);
    bool IndexIsActive (uint32_t index) const;
    void IndexSetActive (uint32_t& index, bool active);

    template <typename TCallback, typename... Args, std::size_t... Seq>
    void UnpackAndCallback (Entity& entity, TCallback&& callback, tmp::TypeList<Args...>, tmp::Sequence<Seq...>);

    // Member variables
    ComponentRegistry m_componentRegistry;
    SystemRegistry m_systemRegistry;
    std::unordered_map<EntityId, uint32_t> m_idToEntity;
    std::vector<Entity> m_entities;
    uint32_t m_entityIdCounter{};
    uint32_t m_entityActiveCount{};
};

template <typename TComponent>
TComponent* Entity::GetComponent (const Encosys& encosys) {
    return const_cast<TComponent*>(static_cast<const Entity*>(this)->GetComponent<TComponent>(encosys));
}

template <typename TComponent>
const TComponent* Entity::GetComponent (const Encosys& encosys) const {
    // Retrieve the registered type of the component
    const ComponentTypeId typeId = encosys.GetComponentType<TComponent>().Id();

    // Return nullptr if this entity does not have this component type
    if (!HasComponent(typeId)) {
        return nullptr;
    }

    // Retrieve the storage for this component type
    const auto& storage = encosys.m_componentRegistry.GetStorage<TComponent>();
    return &storage.GetObject(GetComponentIndex(typeId));
}

template <typename TComponent>
ComponentTypeId Encosys::RegisterComponent () {
    return m_componentRegistry.Register<TComponent>();
}

template <typename TComponent, typename... TArgs>
TComponent& Encosys::AddComponent (EntityId e, TArgs&&... args) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();

    // Retrieve the storage for this component type
    auto& storage = m_componentRegistry.GetStorage<TComponent>();

    // Create the component and set the component index for this entity
    uint32_t componentIndex = storage.Create(std::forward<TArgs>(args)...);
    m_entities[entityIter->second].SetComponentIndex(typeId, componentIndex);
    return storage.GetObject(componentIndex);
}

template <typename TComponent>
void Encosys::RemoveComponent (EntityId e) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();

    // Retrieve the storage for this component type
    auto& storage = m_componentRegistry.GetStorage<TComponent>();

    // Find the component index for this entity and destroy the component
    Entity& entity = m_entities[entityIter->second];
    if (entity.HasComponent(typeId)) {
        storage.Destroy(entity.GetComponentIndex(typeId));
        entity.RemoveComponentIndex(typeId);
    }
}

template <typename TComponent>
TComponent* Encosys::GetComponent (EntityId e) {
    return const_cast<TComponent*>(static_cast<const Encosys*>(this)->GetComponent<TComponent>(e));
}

template <typename TComponent>
const TComponent* Encosys::GetComponent (EntityId e) const {
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());
    return m_entities[entityIter->second].GetComponent<TComponent>(*this);
}

template <typename TComponent>
const ComponentType& Encosys::GetComponentType () const {
    return m_componentRegistry.GetType<TComponent>();
}

template <typename TSystem, typename... TArgs>
void Encosys::RegisterSystem (TArgs&&... args) {
    m_systemRegistry.Register<TSystem>(std::forward<TArgs>(args)...);
}

template <typename TCallback>
void Encosys::ForEach (TCallback&& callback) {
    using FTraits = tmp::FunctionTraits<decltype(callback)>;
    static_assert(FTraits::ArgCount > 0, "First callback param must be ecs::Entity.");
    static_assert(std::is_same<std::decay_t<typename FTraits::Arg<0>>, Entity>::value, "First callback param must be ecs::Entity.");
    using FComponentArgs = typename FTraits::Args::RemoveFirst;

    ComponentBitset targetMask{};
    FComponentArgs::ForTypes([this, &targetMask] (auto t) {
        (void)t;
        assert(m_componentRegistry.HasType<TYPE_OF(t)>());
        targetMask.set(m_componentRegistry.GetTypeId<TYPE_OF(t)>());
    });

    for (uint32_t i = 0; i < m_entityActiveCount; ++i) {
        Entity& entity = m_entities[i];
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
void Encosys::UnpackAndCallback (Entity& entity, TCallback&& callback, tmp::TypeList<Args...>, tmp::Sequence<Seq...>) {
    auto params = std::make_tuple(
        entity,
        std::ref(*entity.GetComponent<std::decay_t<Args>>(*this))...
    );
    callback(std::get<Seq>(params)...);
}

} // namespace ecs
