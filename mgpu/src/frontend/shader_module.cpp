
#include <mgpu/mgpu.h>

#include "backend/shader_module.hpp"

extern "C" {

void mgpuShaderModuleDestroy(MGPUShaderModule shader_module) {
  delete (mgpu::ShaderModuleBase*)shader_module;
}

} // extern "C"
