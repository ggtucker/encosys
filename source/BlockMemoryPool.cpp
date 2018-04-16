#include "BlockMemoryPool.h"

#include <cassert>

namespace ecs {

BlockMemoryPool::~BlockMemoryPool () {
    for (uint8_t* block : m_blocks) {
        delete[] block;
    }
}

void BlockMemoryPool::Resize (uint32_t size) {
    Reserve(size);
    m_size = size;
}

void BlockMemoryPool::Reserve (uint32_t capacity) {
    while (m_capacity < capacity) {
        m_blocks.push_back(new uint8_t[m_elementSize * m_blockSize]);
        m_capacity += m_blockSize;
    }
}

uint8_t* BlockMemoryPool::GetData (uint32_t index) {
    assert(index < m_size);
    return m_blocks[index / m_blockSize] + (index % m_blockSize) * m_elementSize;
}

const uint8_t* BlockMemoryPool::GetData (uint32_t index) const {
    assert(index < m_size);
    return m_blocks[index / m_blockSize] + (index % m_blockSize) * m_elementSize;
}

uint32_t BlockMemoryPool::CreateFromCopy (uint32_t index) {
    assert(index < m_size);
    uint32_t newIndex = m_size;
    memcpy(GetData(newIndex), GetData(index), m_elementSize);
    return newIndex;
}

void BlockMemoryPool::Destroy (uint32_t index) {
    assert(index < m_size);
    memset(GetData(index), 0, m_elementSize);
}

}