
#include <mgpu/mgpu.h>

#include "backend/render_target.hpp"

extern "C" {

void mgpuRenderTargetDestroy(MGPURenderTarget render_target) {
  delete (mgpu::RenderTargetBase*)render_target;
}

} // extern "C"
