
#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <memory>

#include "backend/render_device_backend_base.hpp"
#include "common/result.hpp"

namespace mgpu {

class RenderDeviceBackendOGL final : public RenderDeviceBackendBase {
  public:
    static Result<std::unique_ptr<RenderDeviceBackendBase>> Create(SDL_Window* sdl_window);

   ~RenderDeviceBackendOGL() override;

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo* create_info) override;
    void DestroyBuffer(BufferBase* buffer) override;

  private:
    explicit RenderDeviceBackendOGL(SDL_GLContext gl_context);

    SDL_GLContext m_gl_context{};
};

}  // namespace mgpu
