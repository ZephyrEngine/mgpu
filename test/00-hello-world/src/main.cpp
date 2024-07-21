
#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <SDL2/SDL.h>
#include <optional>
#include <vector>

#undef main

#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0)

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* sdl_window = SDL_CreateWindow(
    "test-00-hello-world",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1600,
    900,
    SDL_WINDOW_VULKAN
  );

  MGPUInstance mgpu_instance{};
  MGPU_CHECK(mgpuCreateInstance(MGPU_BACKEND_TYPE_VULKAN, &mgpu_instance));

  uint32_t mgpu_physical_device_count{};
  std::vector<MGPUPhysicalDevice> mgpu_physical_devices{};
  MGPU_CHECK(mgpuInstanceEnumeratePhysicalDevices(mgpu_instance, &mgpu_physical_device_count, nullptr));
  mgpu_physical_devices.resize(mgpu_physical_device_count);
  MGPU_CHECK(mgpuInstanceEnumeratePhysicalDevices(mgpu_instance, &mgpu_physical_device_count, mgpu_physical_devices.data()));

  std::optional<MGPUPhysicalDevice> discrete_gpu{};
  std::optional<MGPUPhysicalDevice> integrated_gpu{};
  std::optional<MGPUPhysicalDevice> virtual_gpu{};

  for(MGPUPhysicalDevice mgpu_physical_device : mgpu_physical_devices) {
    MGPUPhysicalDeviceInfo info{};
    MGPU_CHECK(mgpuPhysicalDeviceGetInfo(mgpu_physical_device, &info));
    switch(info.device_type) {
      case MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:     discrete_gpu = mgpu_physical_device; break;
      case MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: integrated_gpu = mgpu_physical_device; break;
      case MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       virtual_gpu = mgpu_physical_device; break;
    }
  }

  MGPUPhysicalDevice mgpu_physical_device = discrete_gpu.value_or(
    integrated_gpu.value_or(
      virtual_gpu.value_or((MGPUPhysicalDevice)MGPU_NULL_HANDLE)));

  if(mgpu_physical_device == MGPU_NULL_HANDLE) {
    ATOM_PANIC("failed to find a suitable physical device");
  }

  MGPUPhysicalDeviceInfo mgpu_physical_device_info{};
  MGPU_CHECK(mgpuPhysicalDeviceGetInfo(mgpu_physical_device, &mgpu_physical_device_info));
  fmt::print("mgpu device: {} (type = {})\n", mgpu_physical_device_info.device_name, (int)mgpu_physical_device_info.device_type);

  MGPUDevice mgpu_device{};
  MGPU_CHECK(mgpuPhysicalDeviceCreateDevice(mgpu_physical_device, &mgpu_device));

//#if USE_VULKAN == 1
//  MGPU_CHECK(mgpuCreateRenderDevice(MGPU_BACKEND_VULKAN, sdl_window, &mgpu_render_device));
//#else
//  MGPU_CHECK(mgpuCreateRenderDevice(MGPU_BACKEND_OPENGL, sdl_window, &mgpu_render_device));
//#endif
//
//  const MGPUBufferCreateInfo buffer_create_info{
//    .size = 100 * sizeof(u32),
//    .usage = MGPU_BUFFER_USAGE_COPY_DST | MGPU_BUFFER_USAGE_STORAGE_BUFFER,
//    .flags = MGPU_BUFFER_FLAGS_HOST_VISIBLE
//  };
//  MGPUBuffer mgpu_buffer{};
//  MGPU_CHECK(mgpuCreateBuffer(mgpu_render_device, &buffer_create_info, &mgpu_buffer))
//
//  void* mgpu_buffer_address = nullptr;
//  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
//  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);
////  MGPU_CHECK(mgpuFlushBuffer(mgpu_render_device, mgpu_buffer, 1u, 99 * sizeof(u32)))
////  MGPU_CHECK(mgpuUnmapBuffer(mgpu_render_device, mgpu_buffer))
////  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
////  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);

  SDL_Event sdl_event{};

  while(true) {
    while(SDL_PollEvent(&sdl_event)) {
      if(sdl_event.type == SDL_QUIT) {
        goto done;
      }
    }

//    MGPUFence mgpu_fence = mgpuFenceSync(mgpu_render_device);
//    // ...
//    mgpuWaitFence(mgpu_render_device, mgpu_fence);
  }

done:
//  mgpuDestroyBuffer(mgpu_render_device, mgpu_buffer);
  mgpuDeviceDestroy(mgpu_device);
  mgpuInstanceDestroy(mgpu_instance);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
