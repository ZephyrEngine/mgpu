
#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <SDL2/SDL.h>
#undef main

#define USE_VULKAN 0
#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0);

#ifndef USE_VULKAN

#include <SDL2/SDL_opengl.h>
#include <GL/glew.h>
#include <GL/gl.h>

#endif

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  MGPURenderDevice mgpu_render_device{};

  SDL_Window* sdl_window = SDL_CreateWindow(
    "test-00-hello-world",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1600,
    900,
#if USE_VULKAN == 1
    SDL_WINDOW_VULKAN
#else
    SDL_WINDOW_OPENGL
#endif
  );

#if USE_VULKAN == 1
  MGPU_CHECK(mgpuCreateRenderDevice(MGPU_BACKEND_VULKAN, sdl_window, &mgpu_render_device));
#else
  MGPU_CHECK(mgpuCreateRenderDevice(MGPU_BACKEND_OPENGL, sdl_window, &mgpu_render_device));
#endif

  const MGPUBufferCreateInfo buffer_create_info{
    .size = 100 * sizeof(u32),
    .usage = (MGPUBufferUsage)(MGPU_BUFFER_USAGE_COPY_DST | MGPU_BUFFER_USAGE_STORAGE_BUFFER),
    .flags = MGPU_BUFFER_FLAGS_HOST_VISIBLE,
    .mapped_at_creation = false
  };
  MGPUBuffer mgpu_buffer{};
  MGPU_CHECK(mgpuCreateBuffer(mgpu_render_device, &buffer_create_info, &mgpu_buffer))

  void* mgpu_buffer_address = nullptr;
  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);
//  MGPU_CHECK(mgpuFlushBuffer(mgpu_render_device, mgpu_buffer, 1u, 99 * sizeof(u32)))
//  MGPU_CHECK(mgpuUnmapBuffer(mgpu_render_device, mgpu_buffer))
//  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
//  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);

  SDL_Event sdl_event{};

  while(true) {
    while(SDL_PollEvent(&sdl_event)) {
      if(sdl_event.type == SDL_QUIT) {
        goto done;
      }
    }

#ifndef USE_VULKAN
    SDL_GL_SwapWindow(sdl_window);
#endif
  }

done:
  mgpuDestroyBuffer(mgpu_render_device, mgpu_buffer);
  mgpuDestroyRenderDevice(mgpu_render_device);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
