#pragma once

#include <cstdint>
#include <functional>

namespace ecs {

class Encosys;

class EntityId {
public:
    EntityId () = default;
    explicit operator uint32_t () const { return m_id; }
    uint32_t Id () const { return m_id; }

    friend bool operator== (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id == rhs.m_id; }
    friend bool operator!= (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id != rhs.m_id; }
    friend bool operator<  (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id < rhs.m_id; }
    friend bool operator>  (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id > rhs.m_id; }
    friend bool operator<= (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id <= rhs.m_id; }
    friend bool operator>= (const EntityId& lhs, const EntityId& rhs) { return lhs.m_id >= rhs.m_id; }

private:
    friend class Encosys;
    explicit EntityId (uint32_t id) : m_id{id} {}
    uint32_t m_id{static_cast<uint32_t>(-1)};
};

const EntityId c_invalidEntityId = {};

}

namespace std {
    template <>
    struct hash<ecs::EntityId> {
        std::size_t operator() (const ecs::EntityId& k) const {
            return std::hash<uint32_t>()(k.Id());
        }
    };
}
