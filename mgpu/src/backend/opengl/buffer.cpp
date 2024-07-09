
#include "buffer.hpp"

namespace mgpu {

// TODO(fleroviux): handle any GL errors

Result<BufferBase*> BufferOGL::Create(const MGPUBufferCreateInfo& create_info) {
  const auto buffer_size = (GLsizeiptr)create_info.size;
  if((uint64_t)buffer_size < create_info.size) {
    return MGPU_OUT_OF_MEMORY;
  }

  // TODO(fleroviux): evaluate if this makes sense to keep for host visible buffers.
  GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;

  if(create_info.flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE) {
    // TODO(fleroviux): evaluate if OpenGL implementations would benefit from using only MAP_READ or MAP_WRITE when possible.
    // TODO(fleroviux): evaluate if exposing GL_HOST_COHERENT_BIT would be useful.
    flags |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT;
  }

  GLuint gl_buffer{};
  glCreateBuffers(1u, &gl_buffer);
  glNamedBufferStorage(gl_buffer, buffer_size, nullptr, flags);
  return new BufferOGL{create_info, gl_buffer};
}

BufferOGL::BufferOGL(const MGPUBufferCreateInfo& create_info, GLuint gl_buffer)
    : BufferBase{create_info}
    , m_gl_buffer{gl_buffer} {
}

BufferOGL::~BufferOGL() {
  // I'm not sure if we really need to unmap the buffer before destroying it, but it can't hurt much.
  Unmap();
  glDeleteBuffers(1u, &m_gl_buffer);
}

Result<void*> BufferOGL::Map() {
  if(m_map_address == nullptr) {
    m_map_address = glMapNamedBufferRange(m_gl_buffer, 0, (GLsizeiptr)CreateInfo().size, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
  }
  return m_map_address;
}

MGPUResult BufferOGL::Unmap() {
  if(m_map_address != nullptr) {
    m_map_address = nullptr;
    glUnmapNamedBuffer(m_gl_buffer);
    return MGPU_SUCCESS;
  }
  return MGPU_BUFFER_NOT_MAPPED;
}

MGPUResult BufferOGL::Flush(u64 offset, u64 size) {
  if(m_map_address != nullptr) {
    glFlushMappedNamedBufferRange(m_gl_buffer, (GLintptr)offset, (GLsizeiptr)size);
    return MGPU_SUCCESS;
  }
  return MGPU_BUFFER_NOT_MAPPED;
}

}  // namespace mgpu
