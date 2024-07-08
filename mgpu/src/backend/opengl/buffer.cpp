
#include "buffer.hpp"

namespace mgpu {

Result<BufferBase*> BufferOGL::Create(const MGPUBufferCreateInfo& create_info) {
  const auto buffer_size = (GLsizeiptr)create_info.size;
  if((uint64_t)buffer_size < create_info.size) {
    return MGPU_OUT_OF_MEMORY;
  }

  GLuint gl_buffer{};
  glCreateBuffers(1u, &gl_buffer);
  glNamedBufferStorage(gl_buffer, buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
  return new BufferOGL{create_info, gl_buffer};
}

BufferOGL::BufferOGL(const MGPUBufferCreateInfo& create_info, GLuint gl_buffer)
    : BufferBase{create_info}
    , m_gl_buffer{gl_buffer} {
}

BufferOGL::~BufferOGL() {
  glDeleteBuffers(1u, &m_gl_buffer);
}

}  // namespace mgpu
