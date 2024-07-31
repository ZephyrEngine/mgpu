
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

  {
    MGPUPhysicalDeviceInfo info{};
    MGPU_CHECK(mgpuPhysicalDeviceGetInfo(mgpu_physical_device, &info));
    fmt::print("mgpu device: {} (type = {})\n", info.device_name, (int)info.device_type);

    const MGPUPhysicalDeviceLimits& limits = info.limits;
    fmt::print("\tmax_texture_dimension_1d: {}\n", limits.max_texture_dimension_1d);
    fmt::print("\tmax_texture_dimension_2d: {}\n", limits.max_texture_dimension_2d);
    fmt::print("\tmax_texture_dimension_3d: {}\n", limits.max_texture_dimension_3d);
    fmt::print("\tmax_texture_array_layers: {}\n", limits.max_texture_array_layers);
  }

  MGPUDevice mgpu_device{};
  MGPU_CHECK(mgpuPhysicalDeviceCreateDevice(mgpu_physical_device, &mgpu_device));

  const MGPUBufferCreateInfo buffer_create_info{
    .size = 100 * sizeof(u32),
    .usage = MGPU_BUFFER_USAGE_COPY_DST | MGPU_BUFFER_USAGE_STORAGE_BUFFER,
    .flags = MGPU_BUFFER_FLAGS_HOST_VISIBLE
  };
  MGPUBuffer mgpu_buffer{};
  MGPU_CHECK(mgpuDeviceCreateBuffer(mgpu_device, &buffer_create_info, &mgpu_buffer));

  void* mgpu_buffer_address = nullptr;
  MGPU_CHECK(mgpuBufferMap(mgpu_buffer, &mgpu_buffer_address));
  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);

  MGPU_CHECK(mgpuBufferFlushRange(mgpu_buffer, 10u, MGPU_WHOLE_SIZE));

  const MGPUTextureCreateInfo texture_create_info{
    .format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB,
    .type = MGPU_TEXTURE_TYPE_2D,
    .extent = {
      .width = 512u,
      .height = 512u,
      .depth = 1u
    },
    .mip_count = 1u,
    .array_layer_count = 6u,
    .usage = MGPU_TEXTURE_USAGE_COPY_SRC | MGPU_TEXTURE_USAGE_COPY_DST | MGPU_TEXTURE_USAGE_SAMPLED
  };
  MGPUTexture mgpu_texture{};
  MGPU_CHECK(mgpuDeviceCreateTexture(mgpu_device, &texture_create_info, &mgpu_texture));

  const MGPUTextureViewCreateInfo texture_view_create_info{
    .type = MGPU_TEXTURE_VIEW_TYPE_CUBE,
    .format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB,
    .aspect = MGPU_TEXTURE_ASPECT_COLOR,
    .base_mip = 0u,
    .mip_count = 1u,
    .base_array_layer = 0u,
    .array_layer_count = 6u
  };
  MGPUTextureView mgpu_texture_view{};
  MGPU_CHECK(mgpuTextureCreateView(mgpu_texture, &texture_view_create_info, &mgpu_texture_view));

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
  mgpuTextureViewDestroy(mgpu_texture_view);
  mgpuTextureDestroy(mgpu_texture);
  MGPU_CHECK(mgpuBufferUnmap(mgpu_buffer));
  mgpuBufferDestroy(mgpu_buffer);
  mgpuDeviceDestroy(mgpu_device);
  mgpuInstanceDestroy(mgpu_instance);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
