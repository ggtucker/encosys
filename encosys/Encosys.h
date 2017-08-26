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

// Forward declarations
class Entity;

class EntityStorage {
public:
    explicit EntityStorage        (EntityId id) : m_id{id} {}

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

class Encosys {
public:
    // Constructors
    Encosys () = default;

    // Entity members
    EntityId                                                      Create               (bool active = true);
    void                                                          Destroy              (EntityId e);
    bool                                                          IsValid              (EntityId e) const;
    bool                                                          IsActive             (EntityId e) const;
    void                                                          SetActive            (EntityId e, bool active);
    uint32_t                                                      EntityCount          () const;

    // Component members
    template <typename TComponent> ComponentTypeId                RegisterComponent    ();
    template <typename TComponent, typename... TArgs> TComponent& AddComponent         (EntityId e, TArgs&&... args);
    template <typename TComponent> void                           RemoveComponent      (EntityId e);
    template <typename TComponent> TComponent*                    GetComponent         (EntityId e);
    template <typename TComponent> const TComponent*              GetComponent         (EntityId e) const;
    template <typename TComponent> ComponentTypeId                GetComponentTypeId   () const;

    // System members
    template <typename TSystem, typename... TArgs>   void         RegisterSystem       (TArgs&&... args);
    template <typename TComponent> void                           SetRequiredComponent (SystemType& type, ComponentUsage usage);
    template <typename TComponent> void                           SetOptionalComponent (SystemType& type, ComponentUsage usage);
    void                                                          Initialize           ();
    void                                                          Update               (TimeDelta delta);

    // Other members
    template <typename TCallback> void                            ForEach              (TCallback&& callback);

private:
    friend class Entity;
    // Helper members
    void IndexSwapEntities (uint32_t lhsIndex, uint32_t rhsIndex);
    bool IndexIsActive (uint32_t index) const;
    void IndexSetActive (uint32_t& index, bool active);

    template <typename TCallback, typename... Args, std::size_t... Seq>
    void UnpackAndCallback (const EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>);

    // Member variables
    ComponentRegistry m_componentRegistry;
    SystemRegistry m_systemRegistry;
    std::unordered_map<EntityId, uint32_t> m_idToEntity;
    std::vector<EntityStorage> m_entities;
    uint32_t m_entityIdCounter{};
    uint32_t m_entityActiveCount{};
};

class Entity {
public:
    Entity (Encosys& encosys, const EntityStorage& entityStorage) :
        m_encosys{encosys},
        m_entityStorage{entityStorage},
        m_systemType{nullptr} {
    }

    Entity (Encosys& encosys, const EntityStorage& entityStorage, const SystemType& systemType) :
        m_encosys{encosys},
        m_entityStorage{entityStorage},
        m_systemType{&systemType} {
    }

    EntityId GetId () const {
        return m_entityStorage.GetId();
    }

    template <typename TComponent>
    TComponent* WriteComponent () const {
        assert(!m_systemType || m_systemType->IsWriteAllowed(m_encosys.GetComponentTypeId<TComponent>()));
        return const_cast<TComponent*>(static_cast<const Entity*>(this)->ReadComponent<TComponent>());
    }

    template <typename TComponent>
    const TComponent* ReadComponent () const {
        assert(!m_systemType || m_systemType->IsReadAllowed(m_encosys.GetComponentTypeId<TComponent>()));

        // Retrieve the registered type of the component
        const ComponentTypeId typeId = m_encosys.m_componentRegistry.GetTypeId<TComponent>();

        // Retrieve the storage for this component type
        auto& storage = m_encosys.m_componentRegistry.GetStorage<TComponent>();

        // Find the component index for this entity and return the component
        if (!m_entityStorage.HasComponent(typeId)) {
            return nullptr;
        }
        return &storage.GetObject(m_entityStorage.GetComponentIndex(typeId));
    }

private:
    Encosys& m_encosys;
    const EntityStorage& m_entityStorage;
    const SystemType* m_systemType;
};

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
    EntityStorage& entity = m_entities[entityIter->second];
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
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    assert(entityIter != m_idToEntity.end());

    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_componentRegistry.GetTypeId<TComponent>();

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
ComponentTypeId Encosys::GetComponentTypeId () const {
    return m_componentRegistry.GetTypeId<TComponent>();
}

template <typename TSystem, typename... TArgs>
void Encosys::RegisterSystem (TArgs&&... args) {
    m_systemRegistry.Register<TSystem>(std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Encosys::SetRequiredComponent (SystemType& type, ComponentUsage usage) {
    type.UseComponent(m_componentRegistry.GetTypeId<TComponent>(), ecs::ComponentRequirement::Required, usage);
}

template <typename TComponent>
void Encosys::SetOptionalComponent (SystemType& type, ComponentUsage usage) {
    type.UseComponent(m_componentRegistry.GetTypeId<TComponent>(), ecs::ComponentRequirement::Optional, usage);
}

template <typename TCallback>
void Encosys::ForEach (TCallback&& callback) {
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
void Encosys::UnpackAndCallback (const EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>) {
    auto params = std::make_tuple(
        ecs::Entity(*this, entity),
        std::ref(m_componentRegistry.GetStorage<Args>().GetObject(entity.GetComponentIndex(m_componentRegistry.GetTypeId<Args>())))...
    );
    callback(std::get<Seq>(params)...);
}

} // namespace ecs
