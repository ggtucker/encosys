#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>

#ifndef ENCOSYS_DISABLE_INCLUDE_ECSCONFIG_H
#include "ecsconfig.h"
#endif

#ifndef ENCOSYS_MAX_COMPONENTS_
#define ENCOSYS_MAX_COMPONENTS_ 64
#endif

#ifndef ENCOSYS_MAX_SINGLETONS_
#define ENCOSYS_MAX_SINGLETONS_ 32
#endif

#ifndef ENCOSYS_TIME_TYPE_
#define ENCOSYS_TIME_TYPE_ float
#endif

#ifndef ENCOSYS_ASSERT_
#define ENCOSYS_ASSERT_(x) assert(x)
#endif

namespace ecs {

using ComponentTypeId = uint32_t;
using SingletonTypeId = uint32_t;
using SystemTypeId = uint32_t;
using TimeDelta = ENCOSYS_TIME_TYPE_;

using ComponentBitset = std::bitset<ENCOSYS_MAX_COMPONENTS_>;
using SingletonBitset = std::bitset<ENCOSYS_MAX_SINGLETONS_>;

const uint32_t c_invalidIndex = static_cast<uint32_t>(-1);

}