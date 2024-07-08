
#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <memory>

#include "common/result.hpp"
#include "frontend/buffer.hpp"

namespace mgpu {

class BufferOGL final : public BufferBase {
  public:
    static Result<BufferBase*> Create(const MGPUBufferCreateInfo& create_info);

   ~BufferOGL() override;

   [[nodiscard]] GLuint Handle() {
     return m_gl_buffer;
   }

  private:
    BufferOGL(const MGPUBufferCreateInfo& create_info, GLuint gl_buffer);

    GLuint m_gl_buffer;
};

}  // namespace mgpu
