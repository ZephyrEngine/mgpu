

#include <mgpu/mgpu.h>

#include "backend/resource_set.hpp"

extern "C" {

void mgpuResourceSetDestroy(MGPUResourceSet resource_set) {
  delete (mgpu::ResourceSetBase*)resource_set;
}

} // extern "C"
