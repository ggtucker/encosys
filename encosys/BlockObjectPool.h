#pragma once

#include "BlockMemoryPool.h"
#include <cassert>

namespace ecs {

template <typename T>
class BlockObjectPool : public BlockMemoryPool {
public:
    explicit BlockObjectPool (uint32_t blockSize = 4096)
        : BlockMemoryPool(sizeof(T), blockSize) {}

    uint32_t GetSize () const { return m_size; }

    template <typename... Args>
    uint32_t Create (Args&&... args) {
        uint32_t index;
        if (!m_freeIndices.empty()) {
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
        }
        else {
            index = m_size;
            ++m_size;
            Reserve(m_size);
        }
        new (Get(index)) T(std::forward<Args>(args)...);
        return index;
    }

    T& GetObject (uint32_t index) {
        assert(index < m_size);
        return *static_cast<T*>(BlockMemoryPool::Get(index));
    }

    const T& GetObject (uint32_t index) const {
        assert(index < m_size);
        return *static_cast<const T*>(BlockMemoryPool::Get(index));
    }

    // Must not destroy the same index more than once
    void Destroy (uint32_t index) override {
        assert(index < m_size);
        GetObject(index).~T();
        m_freeIndices.push_back(index);
    }

private:
    uint32_t m_size{0};
    std::vector<uint32_t> m_freeIndices{};
};

}