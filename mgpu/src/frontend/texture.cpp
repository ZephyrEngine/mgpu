
#include <mgpu/mgpu.h>

#include "backend/texture.hpp"

extern "C" {

MGPUResult mgpuTextureCreateView(MGPUTexture texture, const MGPUTextureViewCreateInfo* create_info, MGPUTextureView* texture_view) {
  return MGPU_INTERNAL_ERROR;
}

void mgpuTextureDestroy(MGPUTexture texture) {
  delete (mgpu::TextureBase*)texture;
}

}  // extern "C"
