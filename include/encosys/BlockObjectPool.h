#pragma once

#include "BlockMemoryPool.h"
#include <cassert>

namespace ecs {

template <typename T>
class BlockObjectPool : public BlockMemoryPool {
public:
    explicit BlockObjectPool (uint32_t blockSize = 4096)
        : BlockMemoryPool(sizeof(T), blockSize) {}

    template <typename... Args>
    uint32_t Create (Args&&... args) {
        uint32_t index;
        if (!m_freeIndices.empty()) {
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
        }
        else {
            index = GetSize();
            Resize(GetSize() + 1);
        }
        new (GetData(index)) T(std::forward<Args>(args)...);
        return index;
    }

    T& GetObject (uint32_t index) {
        return *reinterpret_cast<T*>(BlockMemoryPool::GetData(index));
    }

    const T& GetObject (uint32_t index) const {
        return *reinterpret_cast<const T*>(BlockMemoryPool::GetData(index));
    }

    uint32_t CreateFromCopy (uint32_t index) override {
        return Create(GetObject(index));
    }

    // Must not destroy the same index more than once
    virtual void Destroy (uint32_t index) override {
        GetObject(index).~T();
        m_freeIndices.push_back(index);
    }

private:
    std::vector<uint32_t> m_freeIndices{};
};

}