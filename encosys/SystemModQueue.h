#pragma once

#include <vector>
#include "Encosys.h"
#include "ComponentType.h"

namespace ecs {

enum class SystemModType : uint8_t {
    Create,
    Copy,
    Destroy,
    SetComponent,
    RemoveComponent,
    SetNewComponent,
    RemoveNewComponent
};

struct SystemModCreate {
    bool m_active{};
};

struct SystemModCopy {
    EntityId m_entityId{};
    bool m_active{};
};

struct SystemModDestroy {
    EntityId m_entityId{};
};

struct SystemModComponent {
    EntityId m_entityId{};
    ComponentTypeId m_componentTypeId{};
};

struct SystemModNewComponent {
    uint32_t m_newEntityId{};
    ComponentTypeId m_componentTypeId{};
};

class SystemModQueue {
public:
    explicit SystemModQueue (Encosys& encosys) : m_encosys{encosys} {}

    void ApplyQueuedMods ();

    uint32_t Create (bool active = true);
    uint32_t Copy (EntityId e, bool active = true);
    void Destroy (EntityId e);

    template <typename TComponent, typename... TArgs>
    TComponent& SetComponent (uint32_t e, TArgs&&... args);

    template <typename TComponent, typename... TArgs>
    TComponent& SetComponent (EntityId e, TArgs&&... args);

    template <typename TComponent>
    void RemoveComponent (uint32_t e);

    template <typename TComponent>
    void RemoveComponent (EntityId e);

private:
    Encosys& m_encosys;
    std::vector<uint8_t> m_queuedModBytes{};
    uint32_t m_newEntityIdCounter{};
};

template <typename TComponent, typename... TArgs>
TComponent& SystemModQueue::SetComponent (uint32_t e, TArgs&&... args) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModNewComponent) + sizeof(TComponent);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::SetNewComponent;
    byteIndex += sizeof(SystemModType);
    SystemModNewComponent& mod = *reinterpret_cast<SystemModNewComponent*>(&m_queuedModBytes[byteIndex]);
    mod.m_newEntityId = e;
    mod.m_componentTypeId = m_encosys.GetComponentType<TComponent>().Id();
    byteIndex += sizeof(SystemModNewComponent);
    TComponent* component = reinterpret_cast<TComponent*>(&m_queuedModBytes[byteIndex]);
    new (component) TComponent(std::forward<TArgs>(args)...);
    return *component;
}

template <typename TComponent, typename... TArgs>
TComponent& SystemModQueue::SetComponent (EntityId e, TArgs&&... args) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModComponent) + sizeof(TComponent);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::SetComponent;
    byteIndex += sizeof(SystemModType);
    SystemModComponent& mod = *reinterpret_cast<SystemModComponent*>(&m_queuedModBytes[byteIndex]);
    mod.m_entityId = e;
    mod.m_componentTypeId = m_encosys.GetComponentType<TComponent>().Id();
    byteIndex += sizeof(SystemModComponent);
    TComponent* component = reinterpret_cast<TComponent*>(&m_queuedModBytes[byteIndex]);
    new (component) TComponent(std::forward<TArgs>(args)...);
    return *component;
}

template <typename TComponent>
void SystemModQueue::RemoveComponent (uint32_t e) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModNewComponent);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::RemoveNewComponent;
    byteIndex += sizeof(SystemModType);
    SystemModNewComponent& mod = *reinterpret_cast<SystemModNewComponent*>(&m_queuedModBytes[byteIndex]);
    mod.m_newEntityId = e;
    mod.m_componentTypeId = m_encosys.GetComponentType<TComponent>().Id();
}

template <typename TComponent>
void SystemModQueue::RemoveComponent (EntityId e) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModComponent);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::RemoveComponent;
    byteIndex += sizeof(SystemModType);
    SystemModComponent& mod = *reinterpret_cast<SystemModComponent*>(&m_queuedModBytes[byteIndex]);
    mod.m_entityId = e;
    mod.m_componentTypeId = m_encosys.GetComponentType<TComponent>().Id();
}

}