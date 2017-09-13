#include "SystemModQueue.h"

#include <map>

namespace ecs {

void SystemModQueue::ApplyQueuedMods () {
    std::map<uint32_t, EntityId> newEntityMap;
    uint32_t newEntityIdCounter = 0;
    uint32_t byteIndex = 0;
    while (byteIndex < m_queuedModBytes.size()) {
        const SystemModType type = *reinterpret_cast<const SystemModType*>(&m_queuedModBytes[byteIndex]);
        byteIndex += sizeof(SystemModType);
        switch (type) {
            case SystemModType::Create: {
                const SystemModCreate& mod = *reinterpret_cast<const SystemModCreate*>(&m_queuedModBytes[byteIndex]);
                newEntityMap[newEntityIdCounter] = m_encosys.Create(mod.m_active);
                ++newEntityIdCounter;
                byteIndex += sizeof(SystemModCreate);
            } break;
            case SystemModType::Copy: {
                const SystemModCopy& mod = *reinterpret_cast<const SystemModCopy*>(&m_queuedModBytes[byteIndex]);
                if (m_encosys.IsValid(mod.m_entityId)) {
                    newEntityMap[newEntityIdCounter] = m_encosys.Copy(mod.m_entityId, mod.m_active);
                }
                ++newEntityIdCounter;
                byteIndex += sizeof(SystemModCopy);
            } break;
            case SystemModType::Destroy: {
                const SystemModDestroy& mod = *reinterpret_cast<const SystemModDestroy*>(&m_queuedModBytes[byteIndex]);
                if (m_encosys.IsValid(mod.m_entityId)) {
                    m_encosys.Destroy(mod.m_entityId);
                }
                byteIndex += sizeof(SystemModDestroy);
            } break;
            case SystemModType::SetComponent: {
                const SystemModComponent& mod = *reinterpret_cast<const SystemModComponent*>(&m_queuedModBytes[byteIndex]);
                byteIndex += sizeof(SystemModComponent);
                const ComponentType& componentType = m_encosys.GetComponentType(mod.m_componentTypeId);
                if (m_encosys.IsValid(mod.m_entityId)) {
                    uint8_t* componentMemory = m_encosys.GetComponent(mod.m_entityId, mod.m_componentTypeId);
                    if (componentMemory == nullptr) {
                        componentMemory = m_encosys.AddComponent(mod.m_entityId, mod.m_componentTypeId);
                    }
                    assert(componentMemory != nullptr);
                    memcpy(componentMemory, &m_queuedModBytes[byteIndex], componentType.Bytes());
                }
                byteIndex += componentType.Bytes();
            } break;
            case SystemModType::RemoveComponent: {
                const SystemModComponent& mod = *reinterpret_cast<const SystemModComponent*>(&m_queuedModBytes[byteIndex]);
                if (m_encosys.IsValid(mod.m_entityId)) {
                    m_encosys.RemoveComponent(mod.m_entityId, mod.m_componentTypeId);
                }
                byteIndex += sizeof(SystemModComponent);
            } break;
            case SystemModType::SetNewComponent: {
                const SystemModNewComponent& mod = *reinterpret_cast<const SystemModNewComponent*>(&m_queuedModBytes[byteIndex]);
                byteIndex += sizeof(SystemModNewComponent);
                const ComponentType& componentType = m_encosys.GetComponentType(mod.m_componentTypeId);
                auto entityId = newEntityMap.find(mod.m_newEntityId);
                if (entityId != newEntityMap.end() && m_encosys.IsValid(entityId->second)) {
                    uint8_t* componentMemory = m_encosys.GetComponent(entityId->second, mod.m_componentTypeId);
                    if (componentMemory == nullptr) {
                        componentMemory = m_encosys.AddComponent(entityId->second, mod.m_componentTypeId);
                    }
                    assert(componentMemory != nullptr);
                    memcpy(componentMemory, &m_queuedModBytes[byteIndex], componentType.Bytes());
                }
                byteIndex += componentType.Bytes();
            } break;
            case SystemModType::RemoveNewComponent: {
                const SystemModNewComponent& mod = *reinterpret_cast<const SystemModNewComponent*>(&m_queuedModBytes[byteIndex]);
                auto entityId = newEntityMap.find(mod.m_newEntityId);
                if (entityId != newEntityMap.end() && m_encosys.IsValid(entityId->second)) {
                    m_encosys.RemoveComponent(entityId->second, mod.m_componentTypeId);
                }
                byteIndex += sizeof(SystemModComponent);
            } break;
            default: {
                // ERROR: cannot have an invalid system mod type
                assert(false);
            }
        }
    }
    m_queuedModBytes.clear();
    m_newEntityIdCounter = 0;
}

uint32_t SystemModQueue::Create (bool active) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModCreate);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::Create;
    byteIndex += sizeof(SystemModType);
    SystemModCreate& mod = *reinterpret_cast<SystemModCreate*>(&m_queuedModBytes[byteIndex]);
    mod.m_active = active;
    uint32_t id = m_newEntityIdCounter;
    ++m_newEntityIdCounter;
    return id;
}

uint32_t SystemModQueue::Copy (EntityId e, bool active) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModCopy);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::Copy;
    byteIndex += sizeof(SystemModType);
    SystemModCopy& mod = *reinterpret_cast<SystemModCopy*>(&m_queuedModBytes[byteIndex]);
    mod.m_entityId = e;
    mod.m_active = active;
    uint32_t id = m_newEntityIdCounter;
    ++m_newEntityIdCounter;
    return id;
}

void SystemModQueue::Destroy (EntityId e) {
    static constexpr uint32_t s_modSize = sizeof(SystemModType) + sizeof(SystemModDestroy);
    size_t byteIndex = m_queuedModBytes.size();
    m_queuedModBytes.resize(byteIndex + s_modSize);
    *reinterpret_cast<SystemModType*>(&m_queuedModBytes[byteIndex]) = SystemModType::Destroy;
    byteIndex += sizeof(SystemModType);
    SystemModDestroy& mod = *reinterpret_cast<SystemModDestroy*>(&m_queuedModBytes[byteIndex]);
    mod.m_entityId = e;
}

}