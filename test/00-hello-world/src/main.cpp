
#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <SDL2/SDL.h>
#undef main

#define USE_VULKAN 1
#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0);

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
    .usage = (MGPUBufferUsage)(MGPU_BUFFER_USAGE_COPY_DST | MGPU_BUFFER_USAGE_STORAGE_BUFFER)
  };
  MGPUBuffer mgpu_buffer{};
  MGPU_CHECK(mgpuCreateBuffer(mgpu_render_device, &buffer_create_info, &mgpu_buffer))

  SDL_Event sdl_event{};

  while(true) {
    while(SDL_PollEvent(&sdl_event)) {
      if(sdl_event.type == SDL_QUIT) {
        goto done;
      }
    }

    // ...
  }

done:
  mgpuDestroyBuffer(mgpu_render_device, mgpu_buffer);
  mgpuDestroyRenderDevice(mgpu_render_device);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
