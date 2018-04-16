#pragma once

#include "ComponentRegistry.h"
#include "EncosysConfig.h"
#include "EntityId.h"
#include "FunctionTraits.h"
#include "SingletonRegistry.h"
#include "SystemRegistry.h"
#include <array>
#include <unordered_map>
#include <vector>

namespace ecs {

class EntityStorage {
public:
    explicit EntityStorage        (EntityId id) : m_id{id} {}

    EntityId GetId                () const { return m_id; }

    bool     HasComponent         (ComponentTypeId typeId) const { return m_bitset[typeId]; }
    bool     HasComponentBitset   (const ComponentBitset& bitset) const { return (m_bitset & bitset) == bitset; }

    uint32_t GetComponentIndex    (ComponentTypeId typeId) const { return m_components[typeId]; }
    void     SetComponentIndex    (ComponentTypeId typeId, uint32_t index) { m_bitset.set(typeId); m_components[typeId] = index; }
    void     RemoveComponentIndex (ComponentTypeId typeId) { m_bitset.set(typeId, false); m_components[typeId] = c_invalidIndex; }

private:
    EntityId m_id;
    ComponentBitset m_bitset;
    std::array<uint32_t, ENCOSYS_MAX_COMPONENTS_> m_components;
};

class Entity {
public:
    Entity (Encosys* encosys, EntityStorage* storage) : m_encosys{encosys}, m_storage{storage} {}

    bool                                              IsValid            () const { return m_storage != nullptr; }
    EntityId                                          GetId              () const { return m_storage->GetId(); }

    bool                                              HasComponentBitset (const ComponentBitset& bitset) const { return m_storage->HasComponentBitset(bitset); }
    template <typename TComponent> bool               HasComponent       () const { return m_storage->HasComponent(m_encosys->GetComponentTypeId<TComponent>().Id()); }

    template <typename TComponent, typename... TArgs> TComponent& AddComponent (TArgs&&... args) { return m_encosys->AddComponent<TComponent>(GetId(), std::forward<TArgs>(args)...); }
    template <typename TComponent> TComponent*        GetComponent       ();
    template <typename TComponent> const TComponent*  GetComponent       () const;
    template <typename TComponent> ComponentTypeId    GetComponentTypeId () const { return m_encosys->GetComponentTypeId<TComponent>(); }

private:
    Encosys* m_encosys;
    EntityStorage* m_storage{nullptr};
};

class Encosys {
public:
    // Constructors
    Encosys () = default;

    // Entity members
    Entity                                                        Create               (bool active = true);
    EntityId                                                      Copy                 (EntityId e, bool active = true);
    Entity                                                        Get                  (EntityId e);
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
    template <typename TComponent> ComponentTypeId                GetComponentTypeId   () const;

    // Singleton members
    template <typename TSingleton> SingletonTypeId                RegisterSingleton    ();
    template <typename TSingleton> TSingleton&                    GetSingleton         ();
    template <typename TSingleton> const TSingleton&              GetSingleton         () const;
    template <typename TSingleton> SingletonTypeId                GetSingletonTypeId   () const;

    // System members
    template <typename TSystem> void                              RegisterSystem       ();
    const SystemType&                                             GetSystemType        (SystemTypeId systemId) const;

    // Core members
    void                                                          Initialize           ();
    void                                                          Update               (TimeDelta delta);

    // Other members
    template <typename TCallback> void                            ForEach              (TCallback&& callback);
    Entity                                                        operator[]           (uint32_t index) { return Entity(this, &m_entities[index]); }

private:
    friend class Entity;
    // Helper members
    void IndexSwapEntities (uint32_t lhsIndex, uint32_t rhsIndex);
    bool IndexIsActive (uint32_t index) const;
    void IndexSetActive (uint32_t& index, bool active);

    template <typename TCallback, typename... Args, std::size_t... Seq>
    void UnpackAndCallback (EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>);

    // Member variables
    ComponentRegistry m_componentRegistry;
    SingletonRegistry m_singletonRegistry;
    SystemRegistry m_systemRegistry;
    std::unordered_map<EntityId, uint32_t> m_idToEntity;
    std::vector<EntityStorage> m_entities;
    uint32_t m_entityIdCounter{};
    uint32_t m_entityActiveCount{};
};

template <typename TComponent>
TComponent* Entity::GetComponent () {
    return const_cast<TComponent*>(static_cast<const Entity*>(this)->GetComponent<TComponent>());
}

template <typename TComponent>
const TComponent* Entity::GetComponent () const {
    // Retrieve the registered type of the component
    const ComponentTypeId typeId = m_encosys->GetComponentTypeId<TComponent>();

    // Return nullptr if this entity does not have this component type
    if (!m_storage->HasComponent(typeId)) {
        return nullptr;
    }

    // Retrieve the storage for this component type
    const auto& storage = m_encosys->m_componentRegistry.GetStorage<TComponent>();
    return &storage.GetObject(m_storage->GetComponentIndex(typeId));
}

template <typename TComponent>
ComponentTypeId Encosys::RegisterComponent () {
    return m_componentRegistry.Register<TComponent>();
}

template <typename TComponent, typename... TArgs>
TComponent& Encosys::AddComponent (EntityId e, TArgs&&... args) {
    // Verify this entity exists
    auto entityIter = m_idToEntity.find(e);
    ENCOSYS_ASSERT_(entityIter != m_idToEntity.end());

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
    ENCOSYS_ASSERT_(entityIter != m_idToEntity.end());

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
    auto entityIter = m_idToEntity.find(e);
    ENCOSYS_ASSERT_(entityIter != m_idToEntity.end());
    return m_entities[entityIter->second].GetComponent<TComponent>(*this);
}

template <typename TComponent>
ComponentTypeId Encosys::GetComponentTypeId () const {
    return m_componentRegistry.GetTypeId<TComponent>();
}

template <typename TSingleton>
SingletonTypeId Encosys::RegisterSingleton () {
    return m_singletonRegistry.Register<TSingleton>();
}

template <typename TSingleton>
TSingleton& Encosys::GetSingleton () {
    return m_singletonRegistry.GetSingleton<TSingleton>();
}

template <typename TSingleton>
const TSingleton& Encosys::GetSingleton () const {
    return m_singletonRegistry.GetSingleton<TSingleton>();
}

template <typename TSingleton>
SingletonTypeId Encosys::GetSingletonTypeId () const {
    return m_singletonRegistry.GetTypeId<TSingleton>();
}

template <typename TSystem>
void Encosys::RegisterSystem () {
    m_systemRegistry.Register<TSystem>(*this);
}

template <typename TCallback>
void Encosys::ForEach (TCallback&& callback) {
    using FTraits = FunctionTraits<decltype(callback)>;
    static_ENCOSYS_ASSERT_(FTraits::ArgCount > 0, "First callback param must be ecs::Entity.");
    static_ENCOSYS_ASSERT_(std::is_same<std::decay_t<typename FTraits::Arg<0>>, EntityStorage>::value, "First callback param must be ecs::EntityStorage.");
    using FComponentArgs = typename FTraits::Args::RemoveFirst;

    ComponentBitset targetMask{};
    FComponentArgs::ForTypes([this, &targetMask] (auto t) {
        (void)t;
        ENCOSYS_ASSERT_(m_componentRegistry.HasType<TYPE_OF(t)>());
        targetMask.set(m_componentRegistry.GetTypeId<TYPE_OF(t)>());
    });

    for (uint32_t i = 0; i < m_entityActiveCount; ++i) {
        EntityStorage& entity = m_entities[i];
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
void Encosys::UnpackAndCallback (EntityStorage& entity, TCallback&& callback, TypeList<Args...>, Sequence<Seq...>) {
    auto params = std::make_tuple(
        Entity(*this, entity),
        std::ref(m_componentRegistry.GetStorage<std::decay_t<Args>>().GetObject(entity.GetComponentIndex(m_componentRegistry.GetTypeId<std::decay_t<Args>>())))...
    );
    callback(std::get<Seq>(params)...);
}

} // namespace ecs
