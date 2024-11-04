
#pragma once

#include <mgpu/mgpu.h>

inline MGPUResult validate_shader_program_stages(const MGPUShaderProgramCreateInfo* create_info) {
  int shader_stage_bitset = 0;

  for(size_t i = 0; i < create_info->shader_stage_count; i++) {
    const MGPUShaderStageCreateInfo& shader_stage = create_info->shader_stages[i];
    if(shader_stage_bitset & shader_stage.stage) {
      return MGPU_INVALID_ARGUMENT;
    }
    shader_stage_bitset |= shader_stage.stage;
  }

  if(shader_stage_bitset & MGPU_SHADER_STAGE_COMPUTE) {
    if((shader_stage_bitset & ~MGPU_SHADER_STAGE_COMPUTE) != 0) {
      return MGPU_INVALID_ARGUMENT;
    }
  } else if(shader_stage_bitset & MGPU_SHADER_STAGE_VERTEX) {
    if((shader_stage_bitset & ~MGPU_SHADER_STAGE_ALL_GRAPHICS) != 0) {
      return MGPU_INVALID_ARGUMENT;
    }
  } else {
    return MGPU_INVALID_ARGUMENT;
  }

  return MGPU_SUCCESS;
}