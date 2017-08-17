#include "BlockMemoryPool.h"

#include <cassert>

namespace ecs {

BlockMemoryPool::~BlockMemoryPool () {
    for (uint8_t* block : m_blocks) {
        delete[] block;
    }
}

void BlockMemoryPool::Reserve (uint32_t capacity) {
    while (m_capacity < capacity) {
        m_blocks.push_back(new uint8_t[m_elementSize * m_blockSize]);
        m_capacity += m_blockSize;
    }
}

void* BlockMemoryPool::Get (uint32_t index) {
    assert(index < m_capacity);
    return m_blocks[index / m_blockSize] + (index % m_blockSize) * m_elementSize;
}

const void* BlockMemoryPool::Get (uint32_t index) const {
    assert(index < m_capacity);
    return m_blocks[index / m_blockSize] + (index % m_blockSize) * m_elementSize;
}

void BlockMemoryPool::Destroy (uint32_t index) {
    assert(index < m_capacity);
    memset(Get(index), 0, m_elementSize);
}

}