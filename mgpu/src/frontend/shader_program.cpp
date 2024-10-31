
#include <mgpu/mgpu.h>

#include "backend/shader_program.hpp"

extern "C" {

void mgpuShaderProgramDestroy(MGPUShaderProgram shader_program) {
  delete (mgpu::ShaderProgramBase*)shader_program;
}

} // extern "C"
