
#pragma once

#include <mgpu/mgpu.h>

#include "backend/shader_program.hpp"

namespace mgpu::vulkan {

class ShaderProgram : public ShaderProgramBase {
  public:
    explicit ShaderProgram(const MGPUShaderProgramCreateInfo& create_info);

    [[nodiscard]] const MGPUShaderProgramCreateInfo& GetCreateInfo() const { return m_create_info; }

  private:
    MGPUShaderProgramCreateInfo m_create_info{};
};

} // namespace mgpu::vulkan
