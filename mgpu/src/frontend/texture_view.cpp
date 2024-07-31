
#include <mgpu/mgpu.h>

#include "backend/texture_view.hpp"

extern "C" {

void mgpuTextureViewDestroy(MGPUTextureView texture_view) {
  delete (mgpu::TextureViewBase*)texture_view;
}

}  // extern "C"
