
#pragma once

#include <atom/integer.hpp>
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

    Result<void*> Map();
    MGPUResult Unmap();
    MGPUResult Flush(u64 offset, u64 size);

  private:
    BufferOGL(const MGPUBufferCreateInfo& create_info, GLuint gl_buffer);

    GLuint m_gl_buffer;
    void* m_map_address{nullptr};
};

}  // namespace mgpu
