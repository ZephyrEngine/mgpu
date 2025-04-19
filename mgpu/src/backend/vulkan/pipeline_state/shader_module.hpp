
#pragma once

#include <atom/integer.hpp>
#include <vulkan/vulkan.h>

#include "backend/pipeline_state/shader_module.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device;

class ShaderModule : public ShaderModuleBase {
  public:
   ~ShaderModule() override;

    static Result<ShaderModuleBase*> Create(Device* device, const u32* spirv_code, size_t spirv_byte_size);

    [[nodiscard]] VkShaderModule Handle() { return m_vk_shader_module; }

  private:
    ShaderModule(Device* device, VkShaderModule vk_shader_module);

    Device* m_device;
    VkShaderModule m_vk_shader_module;
};

} // namespace mgpu::vulkan
