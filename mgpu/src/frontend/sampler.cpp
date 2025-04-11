
#include <mgpu/mgpu.h>

#include "backend/sampler.hpp"

extern "C" {

void mgpuSamplerDestroy(MGPUSampler sampler) {
  delete (mgpu::SamplerBase*)sampler;
}

} // extern "C"
