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
    uint32_t Construct (Args&&... args) {
        uint32_t index = BlockMemoryPool::Create();
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
        return Construct(GetObject(index));
    }

    // Must not destroy the same index more than once
    void Destroy (uint32_t index) override {
        GetObject(index).~T();
    }
};

}