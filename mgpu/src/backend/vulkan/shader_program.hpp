
#pragma once

#include <mgpu/mgpu.h>
#include <atom/vector_n.hpp>
#include <span>
#include <vulkan/vulkan.h>

#include "backend/shader_program.hpp"

namespace mgpu::vulkan {

class ShaderProgram final : public ShaderProgramBase {
  public:
    explicit ShaderProgram(const MGPUShaderProgramCreateInfo& create_info);

    [[nodiscard]] std::span<const VkPipelineShaderStageCreateInfo> GetVkShaderStages() const {
      return m_vk_shader_stages;
    }

  private:
    /**
     * There can be up to five shader stages in the traditional rendering pipeline.
     * Since compute pipelines only have a single shader stage, we pick five as the maximum.
     */
    atom::Vector_N<VkPipelineShaderStageCreateInfo, 5> m_vk_shader_stages{};
};

} // namespace mgpu::vulkan
