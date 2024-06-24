
#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <memory>

#include "common/result.hpp"
#include "frontend/buffer.hpp"

namespace mgpu {

class OGLBuffer final : public Buffer {
  public:
    static Result<Buffer*> Create(const MGPUBufferCreateInfo& create_info);

   ~OGLBuffer() override;

   [[nodiscard]] GLuint Handle() {
     return m_gl_buffer;
   }

  private:
    OGLBuffer(const MGPUBufferCreateInfo& create_info, GLuint gl_buffer);

    GLuint m_gl_buffer;
};

}  // namespace mgpu
