
#include <mgpu/mgpu.h>

#include "backend/rasterizer_state.hpp"

extern "C" {

void mgpuRasterizerStateDestroy(MGPURasterizerState rasterizer_state) {
  delete (mgpu::RasterizerStateBase*)rasterizer_state;
}

} // extern "C"
