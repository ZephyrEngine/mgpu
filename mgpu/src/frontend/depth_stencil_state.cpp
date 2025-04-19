
#include <mgpu/mgpu.h>

#include "backend/pipeline_state/depth_stencil_state.hpp"

extern "C" {

void mgpuDepthStencilStateDestroy(MGPUDepthStencilState depth_stencil_state) {
  delete (mgpu::DepthStencilStateBase*)depth_stencil_state;
}

} // extern "C"
