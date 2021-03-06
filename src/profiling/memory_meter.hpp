#pragma once

#include "meter.hpp"

namespace nest {
namespace mc {
namespace util {

meter_ptr make_memory_meter();
meter_ptr make_gpu_memory_meter();

} // namespace util
} // namespace mc
} // namespace nest
