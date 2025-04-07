
#include <mgpu/mgpu.h>

#include "backend/resource_set_layout.hpp"

extern "C" {

void mgpuResourceSetLayoutDestroy(MGPUResourceSetLayout resource_set_layout) {
  delete (mgpu::ResourceSetLayoutBase*)resource_set_layout;
}

} // extern "C"
