
#include <mgpu/mgpu.h>

#include "backend/pipeline_state/shader_module.hpp"

extern "C" {

void mgpuShaderModuleDestroy(MGPUShaderModule shader_module) {
  delete (mgpu::ShaderModuleBase*)shader_module;
}

} // extern "C"
