#include "memory.hpp"

#ifdef __linux__
extern "C" {
    #include <malloc.h>
}
#endif

#ifdef NMC_HAVE_GPU
    #include <cuda_runtime.h>
#endif

namespace nest {
namespace mc {
namespace hw {

#if defined(__linux__)
memory_size_type allocated_memory() {
    auto m = mallinfo();
    return m.hblkhd + m.uordblks;
}
#else
memory_size_type allocated_memory() {
    return -1;
}
#endif

#ifdef NMC_HAVE_GPU
memory_size_type gpu_allocated_memory() {
    std::size_t free;
    std::size_t total;
    auto success = cudaMemGetInfo(&free, &total);

    return success==cudaSuccess? total-free: -1;
}
#else
memory_size_type gpu_allocated_memory() {
    return -1;
}
#endif

} // namespace hw
} // namespace mc
} // namespace nest
