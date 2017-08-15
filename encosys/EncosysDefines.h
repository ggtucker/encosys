#pragma once

#include <cstdint>

#ifndef ENCOSYS_MAX_COMPONENTS_
#define ENCOSYS_MAX_COMPONENTS_     128
#endif

using ComponentTypeIndex = uint32_t;

namespace ecs {

const uint32_t c_invalidIndex = static_cast<uint32_t>(-1);

}