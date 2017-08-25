#pragma once

#include <bitset>
#include <cstdint>

#ifndef ENCOSYS_MAX_COMPONENTS_
#define ENCOSYS_MAX_COMPONENTS_     128
#endif

namespace ecs {

using ComponentTypeId = uint32_t;
using SystemTypeId = uint32_t;

using ComponentBitset = std::bitset<ENCOSYS_MAX_COMPONENTS_>;

const uint32_t c_invalidIndex = static_cast<uint32_t>(-1);

}