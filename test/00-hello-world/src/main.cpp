
#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <SDL.h>
#include <SDL_syswm.h>
#include <optional>
#include <vector>

#include "shader/triangle.frag.h"
#include "shader/triangle.vert.h"

#undef main

#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0)

#ifdef SDL_VIDEO_DRIVER_COCOA
  extern "C" CAMetalLayer* TMP_Cocoa_CreateMetalLayer(NSWindow* ns_window);
#endif

MGPUColor hsv_to_rgb(float h, float s, float v) {
  h *= 6.0f;

  const int h_i = (int)h;
  const float f = h - h_i;
  const float p = v * (1.0f - s);
  const float q = v * (1.0f - s * f);
  const float t = v * (1.0f - s * (1.0f - f));

  switch(h_i) {
    case 1: return {.r = q, .g = v, .b = p, .a = 1.0f};
    case 2: return {.r = p, .g = v, .b = t, .a = 1.0f};
    case 3: return {.r = p, .g = q, .b = v, .a = 1.0f};
    case 4: return {.r = t, .g = p, .b = v, .a = 1.0f};
    case 5: return {.r = v, .g = p, .b = q, .a = 1.0f};
  }

  return {.r = v, .g = t, .b = p, .a = 1.0f};
}

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

  MGPUSurface mgpu_surface{};
  {
    SDL_SysWMinfo wm_info{};
    SDL_GetWindowWMInfo(sdl_window, &wm_info);

    MGPUSurfaceCreateInfo surface_create_info{};

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    surface_create_info.win32 = {
      .hinstance = wm_info.info.win.hinstance,
      .hwnd = wm_info.info.win.window
    }
#elif defined(SDL_VIDEO_DRIVER_COCOA)
    surface_create_info.metal = {
      .metal_layer = TMP_Cocoa_CreateMetalLayer(wm_info.info.cocoa.window)
    };
#else
  #error "Unsupported SDL video driver"
#endif

    MGPU_CHECK(mgpuInstanceCreateSurface(mgpu_instance, &surface_create_info, &mgpu_surface));
  }

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

  MGPUSwapChain mgpu_swap_chain{};
  std::vector<MGPUTexture> mgpu_swap_chain_textures{};
  std::vector<MGPUTextureView> mgpu_swap_chain_texture_views{};

  // Swap Chain creation
  {
    uint32_t surface_format_count{};
    std::vector<MGPUSurfaceFormat> surface_formats{};

    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfaceFormats(mgpu_physical_device, mgpu_surface, &surface_format_count, nullptr));
    surface_formats.resize(surface_format_count);
    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfaceFormats(mgpu_physical_device, mgpu_surface, &surface_format_count, surface_formats.data()));

    fmt::print("supported surface formats:\n");

    bool got_required_surface_format = false;

    for(const MGPUSurfaceFormat& surface_format : surface_formats) {
      fmt::print("\tformat={} color_space={}\n", (int)surface_format.format, (int)surface_format.color_space);

      if(surface_format.format == MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB && surface_format.color_space == MGPU_COLOR_SPACE_SRGB_NONLINEAR) {
        got_required_surface_format = true;
      }
    }

    if(!got_required_surface_format) {
      ATOM_PANIC("Failed to find a suitable surface format");
    }

    uint32_t present_modes_count{};
    std::vector<MGPUPresentMode> present_modes{};

    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfacePresentModes(mgpu_physical_device, mgpu_surface, &present_modes_count, nullptr));
    present_modes.resize(present_modes_count);
    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfacePresentModes(mgpu_physical_device, mgpu_surface, &present_modes_count, present_modes.data()));

    fmt::print("supported present modes:\n");

    for(MGPUPresentMode present_mode : present_modes) {
      fmt::print("\t{}\n", (int)present_mode);
    }

    MGPUSurfaceCapabilities surface_capabilities{};
    MGPU_CHECK(mgpuPhysicalDeviceGetSurfaceCapabilities(mgpu_physical_device, mgpu_surface, &surface_capabilities));
    fmt::print("surface capabilities:\n");
    fmt::print("\tmin_texture_count={}\n", surface_capabilities.min_texture_count);
    fmt::print("\tmax_texture_count={}\n", surface_capabilities.max_texture_count);
    fmt::print("\tcurrent_extent=({}, {})\n", surface_capabilities.current_extent.width, surface_capabilities.current_extent.height);
    fmt::print("\tmin_texture_extent=({}, {})\n", surface_capabilities.min_texture_extent.width, surface_capabilities.min_texture_extent.height);
    fmt::print("\tmax_texture_extent=({}, {})\n", surface_capabilities.max_texture_extent.width, surface_capabilities.max_texture_extent.height);
    fmt::print("\tsupported_usage={}\n", surface_capabilities.supported_usage);

    const MGPUSwapChainCreateInfo swap_chain_create_info{
      .surface = mgpu_surface,
      .format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB,
      .color_space = MGPU_COLOR_SPACE_SRGB_NONLINEAR,
      .present_mode = MGPU_PRESENT_MODE_FIFO,
      .usage = MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
      .extent = surface_capabilities.current_extent,
      .min_texture_count = 2u,
      .old_swap_chain = nullptr
    };
    MGPU_CHECK(mgpuDeviceCreateSwapChain(mgpu_device, &swap_chain_create_info, &mgpu_swap_chain));

    u32 texture_count{};
    MGPU_CHECK(mgpuSwapChainEnumerateTextures(mgpu_swap_chain, &texture_count, nullptr));
    mgpu_swap_chain_textures.resize(texture_count);
    MGPU_CHECK(mgpuSwapChainEnumerateTextures(mgpu_swap_chain, &texture_count, mgpu_swap_chain_textures.data()));

    for(MGPUTexture texture : mgpu_swap_chain_textures) {
      const MGPUTextureViewCreateInfo texture_view_create_info{
        .type = MGPU_TEXTURE_VIEW_TYPE_2D,
        .format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB,
        .aspect = MGPU_TEXTURE_ASPECT_COLOR,
        .base_mip = 0u,
        .mip_count = 1u,
        .base_array_layer = 0u,
        .array_layer_count = 1u
      };

      MGPUTextureView texture_view{};
      MGPU_CHECK(mgpuTextureCreateView(texture, &texture_view_create_info, &texture_view));
      mgpu_swap_chain_texture_views.push_back(texture_view);
    }
  }

  fmt::print("number of swap chain textures: {}\n", mgpu_swap_chain_textures.size());

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

  //  void* mgpu_buffer_address = nullptr;
//  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
//  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);
////  MGPU_CHECK(mgpuFlushBuffer(mgpu_render_device, mgpu_buffer, 1u, 99 * sizeof(u32)))
////  MGPU_CHECK(mgpuUnmapBuffer(mgpu_render_device, mgpu_buffer))
////  MGPU_CHECK(mgpuMapBuffer(mgpu_render_device, mgpu_buffer, &mgpu_buffer_address))
////  fmt::print("Buffer address: 0x{:016X}\n", (u64)mgpu_buffer_address);

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

  MGPUShaderModule mgpu_vert_shader{};
  MGPUShaderModule mgpu_frag_shader{};
  MGPU_CHECK(mgpuDeviceCreateShaderModule(mgpu_device, triangle_vert, sizeof(triangle_vert), &mgpu_vert_shader));
  MGPU_CHECK(mgpuDeviceCreateShaderModule(mgpu_device, triangle_frag, sizeof(triangle_frag), &mgpu_frag_shader));

  MGPUShaderProgramCreateInfo shader_program_create_info{
    .vertex = mgpu_vert_shader,
    .fragment = mgpu_frag_shader
  };
  MGPUShaderProgram mgpu_shader_program{};
  MGPU_CHECK(mgpuDeviceCreateShaderProgram(mgpu_device, &shader_program_create_info, &mgpu_shader_program));

  MGPUCommandList mgpu_cmd_list{};
  MGPU_CHECK(mgpuDeviceCreateCommandList(mgpu_device, &mgpu_cmd_list));

  MGPUQueue mgpu_queue = mgpuDeviceGetQueue(mgpu_device, MGPU_QUEUE_TYPE_ASYNC_COMPUTE);

  SDL_Event sdl_event{};

  float hue = 0.0f;

  while(true) {
    u32 texture_index;
    MGPU_CHECK(mgpuSwapChainAcquireNextTexture(mgpu_swap_chain, &texture_index));

    MGPU_CHECK(mgpuCommandListClear(mgpu_cmd_list));

    const MGPURenderPassColorAttachment render_pass_color_attachments[1] {
      {
        .texture_view = mgpu_swap_chain_texture_views[texture_index],
        .load_op = MGPU_LOAD_OP_CLEAR,
        .store_op = MGPU_STORE_OP_STORE,
        .clear_color = hsv_to_rgb(hue, 1.0f, 1.0f)
      }
    };
    const MGPURenderPassBeginInfo render_pass_info{
      .color_attachment_count = 1u,
      .color_attachments = render_pass_color_attachments,
      .depth_stencil_attachment = nullptr
    };
    mgpuCommandListCmdBeginRenderPass(mgpu_cmd_list, &render_pass_info);
    mgpuCommandListCmdUseShaderProgram(mgpu_cmd_list, mgpu_shader_program);
    mgpuCommandListCmdEndRenderPass(mgpu_cmd_list);

    MGPU_CHECK(mgpuQueueSubmitCommandList(mgpu_queue, mgpu_cmd_list));
    // MGPU_CHECK(mgpuQueueFlush(mgpu_queue));
    MGPU_CHECK(mgpuSwapChainPresent(mgpu_swap_chain));

    hue = std::fmod(hue + 0.0025f, 1.0f);

    while(SDL_PollEvent(&sdl_event)) {
      if(sdl_event.type == SDL_QUIT) {
        goto done;
      }
    }
  }

done:
  mgpuCommandListDestroy(mgpu_cmd_list);
  mgpuShaderProgramDestroy(mgpu_shader_program);
  mgpuShaderModuleDestroy(mgpu_frag_shader);
  mgpuShaderModuleDestroy(mgpu_vert_shader);
  mgpuTextureViewDestroy(mgpu_texture_view);
  mgpuTextureDestroy(mgpu_texture);
  MGPU_CHECK(mgpuBufferUnmap(mgpu_buffer));
  mgpuBufferDestroy(mgpu_buffer);
  for(MGPUTextureView texture_view : mgpu_swap_chain_texture_views) mgpuTextureViewDestroy(texture_view);
  mgpuSwapChainDestroy(mgpu_swap_chain);
  mgpuDeviceDestroy(mgpu_device);
  mgpuSurfaceDestroy(mgpu_surface);
  mgpuInstanceDestroy(mgpu_instance);
  SDL_DestroyWindow(sdl_window);
  return 0;
}
