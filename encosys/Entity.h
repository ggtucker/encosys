#pragma once

#include <cstdint>
#include <functional>

namespace ecs {

class EntitySystem;

class Entity {
public:
    Entity () = default;
    explicit operator uint64_t () const { return m_id; }
    uint64_t Id () const { return m_id; }

    friend bool operator== (const Entity& lhs, const Entity& rhs) { return lhs.m_id == rhs.m_id; }
    friend bool operator!= (const Entity& lhs, const Entity& rhs) { return lhs.m_id != rhs.m_id; }
    friend bool operator<  (const Entity& lhs, const Entity& rhs) { return lhs.m_id < rhs.m_id; }
    friend bool operator>  (const Entity& lhs, const Entity& rhs) { return lhs.m_id > rhs.m_id; }
    friend bool operator<= (const Entity& lhs, const Entity& rhs) { return lhs.m_id <= rhs.m_id; }
    friend bool operator>= (const Entity& lhs, const Entity& rhs) { return lhs.m_id >= rhs.m_id; }

private:
    friend class EntitySystem;
    explicit Entity (uint64_t id) : m_id{id} {}
    uint64_t m_id{static_cast<uint64_t>(-1)};
};

const Entity c_invalidEntity = {};

}

namespace std {
    template <>
    struct hash<ecs::Entity> {
        std::size_t operator() (const ecs::Entity& k) const {
            return std::hash<uint64_t>()(k.Id());
        }
    };
}
