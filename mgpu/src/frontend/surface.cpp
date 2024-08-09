
#include <mgpu/mgpu.h>

#include "backend/surface.hpp"

extern "C" {

void mgpuSurfaceDestroy(MGPUSurface surface) {
  delete (mgpu::SurfaceBase*)surface;
}

}  // extern "C"
