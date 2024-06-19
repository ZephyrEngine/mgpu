
#include <mgpu/mgpu.h>

#include <atom/panic.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0);

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* sdl_window = SDL_CreateWindow(
    "test-00-hello-world",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1600,
    900,
    SDL_WINDOW_OPENGL
  );

  MGPURenderDevice mgpu_render_device{};
  MGPU_CHECK(mgpuCreateRenderDevice(MGPU_BACKEND_OPENGL, sdl_window, &mgpu_render_device));

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
  mgpuDestroyRenderDevice(mgpu_render_device);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
