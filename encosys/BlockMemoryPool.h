#pragma once

#include <vector>

namespace ecs {

class BlockMemoryPool {
public:
    BlockMemoryPool () {}
    explicit BlockMemoryPool (uint32_t elementSize, uint32_t blockSize)
        : m_elementSize{elementSize}, m_blockSize{blockSize} {}

    virtual ~BlockMemoryPool ();

    uint32_t GetElementSize () const { return m_elementSize; }
    uint32_t GetBlockSize () const { return m_blockSize;}
    uint32_t GetCapacity () const { return m_capacity; }
    uint32_t GetSize () const { return m_size; }

    void Resize (uint32_t size);
    void Reserve (uint32_t capacity);

    uint32_t Create ();
    virtual uint32_t CreateFromCopy (uint32_t index);
    virtual void Destroy (uint32_t index);

    uint8_t* GetData (uint32_t index);
    const uint8_t* GetData (uint32_t index) const;

private:
    uint32_t m_elementSize{0};
    uint32_t m_blockSize{0};
    uint32_t m_capacity{0};
    uint32_t m_size{0};
    std::vector<uint8_t*> m_blocks{};
    std::vector<uint32_t> m_freeIndices{};
};

}