
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/device.hpp"
#include "shader_module.hpp"

namespace mgpu::vulkan {

ShaderModule::ShaderModule(Device* device, VkShaderModule vk_shader_module)
    : m_device{device}
    , m_vk_shader_module{vk_shader_module} {
}

ShaderModule::~ShaderModule() {
  VkDevice vk_device = m_device->Handle();
  VkShaderModule vk_shader_module = m_vk_shader_module;

  m_device->GetDeleterQueue().Schedule([vk_device, vk_shader_module]() {
    vkDestroyShaderModule(vk_device, vk_shader_module, nullptr);
  });
}

Result<ShaderModuleBase*> ShaderModule::Create(Device* device, const u32* spirv_code, size_t spirv_byte_size) {
  const VkShaderModuleCreateInfo vk_shader_module_create_info{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .codeSize = spirv_byte_size,
    .pCode = spirv_code
  };

  VkShaderModule vk_shader_module{};
  MGPU_VK_FORWARD_ERROR(vkCreateShaderModule(device->Handle(), &vk_shader_module_create_info, nullptr, &vk_shader_module));
  return new ShaderModule(device, vk_shader_module);
}

} // namespace mgpu::vulkan
