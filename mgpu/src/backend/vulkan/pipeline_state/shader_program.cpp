
#include <vector>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/conversion.hpp"
#include "backend/vulkan/device.hpp"
#include "backend/vulkan/resource_set_layout.hpp"
#include "shader_module.hpp"
#include "shader_program.hpp"

namespace mgpu::vulkan {

ShaderProgram::ShaderProgram(
  Device* device,
  VkPipelineLayout vk_pipeline_layout,
  const MGPUShaderProgramCreateInfo& create_info
)   : m_device{device}
    , m_vk_pipeline_layout{vk_pipeline_layout} {
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

ShaderProgram::~ShaderProgram() {
  Device* device = m_device;
  VkPipelineLayout vk_pipeline_layout = m_vk_pipeline_layout;
  device->GetDeleterQueue().Schedule([device, vk_pipeline_layout]() {
    vkDestroyPipelineLayout(device->Handle(), vk_pipeline_layout, nullptr);
  });
}

Result<ShaderProgramBase*> ShaderProgram::Create(Device* device, const MGPUShaderProgramCreateInfo& create_info) {
  std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts{};
  vk_descriptor_set_layouts.resize(create_info.resource_set_count);

  for(size_t i = 0; i < create_info.resource_set_count; i++) {
    vk_descriptor_set_layouts[i] = ((ResourceSetLayout*)create_info.resource_set_layouts[i])->Handle();
  }

  const VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .setLayoutCount = (u32)vk_descriptor_set_layouts.size(),
    .pSetLayouts = vk_descriptor_set_layouts.data(),
    .pushConstantRangeCount = 0u,
    .pPushConstantRanges = nullptr
  };

  VkPipelineLayout vk_pipeline_layout{};
  MGPU_VK_FORWARD_ERROR(vkCreatePipelineLayout(device->Handle(), &vk_pipeline_layout_create_info, nullptr, &vk_pipeline_layout));
  return new ShaderProgram{device, vk_pipeline_layout, create_info};
}

} // namespace mgpu::vulkan
