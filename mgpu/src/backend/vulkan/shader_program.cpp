
#include "conversion.hpp"
#include "shader_module.hpp"
#include "shader_program.hpp"

namespace mgpu::vulkan {

ShaderProgram::ShaderProgram(const MGPUShaderProgramCreateInfo& create_info) {
  const auto PushShaderStage = [this](MGPUShaderModule shader_module, const char* entrypoint, VkShaderStageFlagBits shader_stage) {
    m_vk_shader_stages.PushBack({
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0u,
      .stage = shader_stage,
      .module = ((ShaderModule*)shader_module)->Handle(),
      .pName = entrypoint,
      .pSpecializationInfo = nullptr
    });
  };

  for(size_t i = 0; i < create_info.shader_stage_count; i++) {
    const MGPUShaderStageCreateInfo& shader_stage = create_info.shader_stages[i];
    PushShaderStage(shader_stage.module, shader_stage.entrypoint, MGPUShaderStageBitToVkShaderStageBit(shader_stage.stage));
  }
}

} // namespace mgpu::vulkan
