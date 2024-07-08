
#include "buffer.hpp"
#include "render_device_backend.hpp"

namespace mgpu {

Result<std::unique_ptr<RenderDeviceBackendBase>> RenderDeviceBackendOGL::Create(SDL_Window* sdl_window) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  const SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
  if(gl_context == nullptr) {
    return MGPU_INTERNAL_ERROR;
  }

  if(glewInit() != GLEW_OK) {
    return MGPU_INTERNAL_ERROR;
  }

  return std::unique_ptr<RenderDeviceBackendBase>{new RenderDeviceBackendOGL{gl_context}};
}

RenderDeviceBackendOGL::RenderDeviceBackendOGL(SDL_GLContext gl_context) : m_gl_context{gl_context} {
}

RenderDeviceBackendOGL::~RenderDeviceBackendOGL() {
  SDL_GL_DeleteContext(m_gl_context);
}

Result<BufferBase*> RenderDeviceBackendOGL::CreateBuffer(const MGPUBufferCreateInfo* create_info) {
  return BufferOGL::Create(*create_info);
}

void RenderDeviceBackendOGL::DestroyBuffer(BufferBase* buffer) {
  delete buffer;
}

}  // namespace mgpu
