
#include <mgpu/mgpu.h>

#include "backend/color_blend_state.hpp"

extern "C" {

void mgpuColorBlendStateDestroy(MGPUColorBlendState color_blend_state) {
  delete (mgpu::ColorBlendStateBase*)color_blend_state;
}

} // extern "C"
