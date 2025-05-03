
#include <atom/math/matrix4.hpp>
#include <atom/integer.hpp>
#include <atom/panic.hpp>

#include <optional>

#include "shader/cube.frag.h"
#include "shader/cube.vert.h"
#include "application.hpp"

#define MGPU_CHECK(result_expression) \
  do { \
    MGPUResult result = result_expression; \
    if(result != MGPU_SUCCESS) \
      ATOM_PANIC("MGPU error: {} ({})", "" # result_expression, mgpuResultCodeToString(result)); \
  } while(0)

#ifdef SDL_VIDEO_DRIVER_COCOA
  extern "C" CAMetalLayer* TMP_Cocoa_CreateMetalLayer(NSWindow* ns_window);
#endif

Application::Application() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* sdl_window = SDL_CreateWindow(
    "test-02-hello-cube",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1600,
    900,
    SDL_WINDOW_VULKAN
  );

  MGPU_CHECK(mgpuCreateInstance(MGPU_BACKEND_TYPE_VULKAN, &m_mgpu_instance));

  {
    SDL_SysWMinfo wm_info{};
    SDL_GetWindowWMInfo(sdl_window, &wm_info);

    MGPUSurfaceCreateInfo surface_create_info{};

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    surface_create_info.win32 = {
      .hinstance = wm_info.info.win.hinstance,
      .hwnd = wm_info.info.win.window
    };
#elif defined(SDL_VIDEO_DRIVER_COCOA)
    surface_create_info.metal = {
      .metal_layer = TMP_Cocoa_CreateMetalLayer(wm_info.info.cocoa.window)
    };
#else
  #error "Unsupported SDL video driver"
#endif

    MGPU_CHECK(mgpuInstanceCreateSurface(m_mgpu_instance, &surface_create_info, &m_mgpu_surface));
  }

  uint32_t mgpu_physical_device_count{};
  std::vector<MGPUPhysicalDevice> mgpu_physical_devices{};
  MGPU_CHECK(mgpuInstanceEnumeratePhysicalDevices(m_mgpu_instance, &mgpu_physical_device_count, nullptr));
  mgpu_physical_devices.resize(mgpu_physical_device_count);
  MGPU_CHECK(mgpuInstanceEnumeratePhysicalDevices(m_mgpu_instance, &mgpu_physical_device_count, mgpu_physical_devices.data()));

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

  MGPU_CHECK(mgpuPhysicalDeviceCreateDevice(mgpu_physical_device, &m_mgpu_device));

  std::vector<MGPUTexture> mgpu_swap_chain_textures{};

  // Swap Chain creation
  {
    uint32_t surface_format_count{};
    std::vector<MGPUSurfaceFormat> surface_formats{};

    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfaceFormats(mgpu_physical_device, m_mgpu_surface, &surface_format_count, nullptr));
    surface_formats.resize(surface_format_count);
    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfaceFormats(mgpu_physical_device, m_mgpu_surface, &surface_format_count, surface_formats.data()));

    bool got_required_surface_format = false;

    for(const MGPUSurfaceFormat& surface_format : surface_formats) {
      if(surface_format.format == MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB && surface_format.color_space == MGPU_COLOR_SPACE_SRGB_NONLINEAR) {
        got_required_surface_format = true;
      }
    }

    if(!got_required_surface_format) {
      ATOM_PANIC("Failed to find a suitable surface format");
    }

    uint32_t present_modes_count{};
    std::vector<MGPUPresentMode> present_modes{};

    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfacePresentModes(mgpu_physical_device, m_mgpu_surface, &present_modes_count, nullptr));
    present_modes.resize(present_modes_count);
    MGPU_CHECK(mgpuPhysicalDeviceEnumerateSurfacePresentModes(mgpu_physical_device, m_mgpu_surface, &present_modes_count, present_modes.data()));

    MGPUSurfaceCapabilities surface_capabilities{};
    MGPU_CHECK(mgpuPhysicalDeviceGetSurfaceCapabilities(mgpu_physical_device, m_mgpu_surface, &surface_capabilities));

    const MGPUSwapChainCreateInfo swap_chain_create_info{
      .surface = m_mgpu_surface,
      .format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB,
      .color_space = MGPU_COLOR_SPACE_SRGB_NONLINEAR,
      .present_mode = MGPU_PRESENT_MODE_FIFO,
      .usage = MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
      .extent = surface_capabilities.current_extent,
      .min_texture_count = 2u,
      .old_swap_chain = nullptr
    };
    MGPU_CHECK(mgpuDeviceCreateSwapChain(m_mgpu_device, &swap_chain_create_info, &m_mgpu_swap_chain));

    u32 texture_count{};
    MGPU_CHECK(mgpuSwapChainEnumerateTextures(m_mgpu_swap_chain, &texture_count, nullptr));
    mgpu_swap_chain_textures.resize(texture_count);
    MGPU_CHECK(mgpuSwapChainEnumerateTextures(m_mgpu_swap_chain, &texture_count, mgpu_swap_chain_textures.data()));

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
      m_mgpu_swap_chain_texture_views.push_back(texture_view);
    }
  }

  const MGPUTextureCreateInfo depth_texture_create_info{
    .format = MGPU_TEXTURE_FORMAT_DEPTH_F32,
    .type = MGPU_TEXTURE_TYPE_2D,
    .extent = {
      .width = 1600u,
      .height = 900u,
      .depth = 1u
    },
    .mip_count = 1u,
    .array_layer_count = 1u,
    .usage = MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT
  };
  MGPU_CHECK(mgpuDeviceCreateTexture(m_mgpu_device, &depth_texture_create_info, &m_mgpu_depth_texture));

  const MGPUTextureViewCreateInfo depth_texture_view_create_info{
    .type = MGPU_TEXTURE_VIEW_TYPE_2D,
    .format = MGPU_TEXTURE_FORMAT_DEPTH_F32,
    .aspect = MGPU_TEXTURE_ASPECT_DEPTH,
    .base_mip = 0u,
    .mip_count = 1u,
    .base_array_layer = 0u,
    .array_layer_count = 1u
  };
  MGPU_CHECK(mgpuTextureCreateView(m_mgpu_depth_texture, &depth_texture_view_create_info, &m_mgpu_depth_texture_view));

  MGPUQueue mgpu_queue = mgpuDeviceGetQueue(m_mgpu_device, MGPU_QUEUE_TYPE_GRAPHICS_COMPUTE);

  const float vertices[] {
    // POSITION         | NORMAL

     // Front
    -1.f, -1.f, -1.f,     0.f,  0.f, -1.f,
     1.f, -1.f, -1.f,     0.f,  0.f, -1.f,
     1.f,  1.f, -1.f,     0.f,  0.f, -1.f,
    -1.f,  1.f, -1.f,     0.f,  0.f, -1.f,

     // Rear
    -1.f, -1.f,  1.f,     0.f,  0.f,  1.f,
     1.f, -1.f,  1.f,     0.f,  0.f,  1.f,
     1.f,  1.f,  1.f,     0.f,  0.f,  1.f,
    -1.f,  1.f,  1.f,     0.f,  0.f,  1.f,

     // Left
    -1.f, -1.f, -1.f,    -1.f,  0.f,  0.f,
    -1.f, -1.f,  1.f,    -1.f,  0.f,  0.f,
    -1.f,  1.f,  1.f,    -1.f,  0.f,  0.f,
    -1.f,  1.f, -1.f,    -1.f,  0.f,  0.f,

     // Right
     1.f, -1.f, -1.f,     1.f,  0.f,  0.f,
     1.f, -1.f,  1.f,     1.f,  0.f,  0.f,
     1.f,  1.f,  1.f,     1.f,  0.f,  0.f,
     1.f,  1.f, -1.f,     1.f,  0.f,  0.f,

     // Top
    -1.f, -1.f, -1.f,     0.f, -1.f,  0.f,
     1.f, -1.f, -1.f,     0.f, -1.f,  0.f,
     1.f, -1.f,  1.f,     0.f, -1.f,  0.f,
    -1.f, -1.f,  1.f,     0.f, -1.f,  0.f,

    // Bottom
   -1.f,  1.f, -1.f,     0.f,  1.f,  0.f,
    1.f,  1.f, -1.f,     0.f,  1.f,  0.f,
    1.f,  1.f,  1.f,     0.f,  1.f,  0.f,
   -1.f,  1.f,  1.f,     0.f,  1.f,  0.f,
  };

  const u16 indices[] {
     0,  1,  2,  2,  3,  0,
     6,  5,  4,  4,  7,  6,
    10,  9,  8,  8, 11, 10,
    12, 13, 14, 14, 15, 12,
    18, 17, 16, 16, 19, 18,
    20, 21, 22, 22, 23, 20
  };

  const MGPUBufferCreateInfo vbo_create_info{
    .size = sizeof(vertices),
    .usage = MGPU_BUFFER_USAGE_VERTEX_BUFFER | MGPU_BUFFER_USAGE_COPY_DST,
    .flags = 0
  };
  MGPU_CHECK(mgpuDeviceCreateBuffer(m_mgpu_device, &vbo_create_info, &m_mgpu_vbo));
  MGPU_CHECK(mgpuQueueBufferUpload(mgpu_queue, m_mgpu_vbo, 0u, sizeof(vertices), vertices));

  const MGPUBufferCreateInfo ibo_create_info{
    .size = sizeof(indices),
    .usage = MGPU_BUFFER_USAGE_INDEX_BUFFER | MGPU_BUFFER_USAGE_COPY_DST,
    .flags = 0
  };
  MGPU_CHECK(mgpuDeviceCreateBuffer(m_mgpu_device, &ibo_create_info, &m_mgpu_ibo));
  MGPU_CHECK(mgpuQueueBufferUpload(mgpu_queue, m_mgpu_ibo, 0u, sizeof(indices), indices));

  const MGPUBufferCreateInfo ubo_create_info{
    .size = 192u, // Fits three 4x4 f32 matrices
    .usage = MGPU_BUFFER_USAGE_UNIFORM_BUFFER | MGPU_BUFFER_USAGE_COPY_DST,
    .flags = 0
  };
  MGPU_CHECK(mgpuDeviceCreateBuffer(m_mgpu_device, &ubo_create_info, &m_mgpu_ubo));

  const atom::Matrix4 projection_matrix = atom::Matrix4::PerspectiveVK(45.f, 1600.f/900.f, 0.01f, 100.f);
  MGPU_CHECK(mgpuQueueBufferUpload(mgpu_queue, m_mgpu_ubo, 0u, sizeof(projection_matrix), &projection_matrix));

  std::vector<MGPUResourceSetLayoutBinding> resource_set_layout_bindings{
    {
      .binding = 0u,
      .type = MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER,
      .visibility = MGPU_SHADER_STAGE_ALL
    }
  };
  const MGPUResourceSetLayoutCreateInfo resource_set_layout_create_info{
    .binding_count = (u32)resource_set_layout_bindings.size(),
    .bindings = resource_set_layout_bindings.data()
  };
  MGPU_CHECK(mgpuDeviceCreateResourceSetLayout(m_mgpu_device, &resource_set_layout_create_info, &m_mgpu_resource_set_layout));

  std::vector<MGPUResourceSetBinding> resource_set_bindings{
    {
      .binding = 0u,
      .type = MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER,
      .buffer = {
        .buffer = m_mgpu_ubo,
        .offset = 0u,
        .size = 192u
      }
    }
  };
  const MGPUResourceSetCreateInfo resource_set_create_info{
    .layout = m_mgpu_resource_set_layout,
    .binding_count = (u32)resource_set_bindings.size(),
    .bindings = resource_set_bindings.data(),
  };
  MGPU_CHECK(mgpuDeviceCreateResourceSet(m_mgpu_device, &resource_set_create_info, &m_mgpu_resource_set));

  MGPU_CHECK(mgpuDeviceCreateShaderModule(m_mgpu_device, cube_vert, sizeof(cube_vert), &m_mgpu_vert_shader));
  MGPU_CHECK(mgpuDeviceCreateShaderModule(m_mgpu_device, cube_frag, sizeof(cube_frag), &m_mgpu_frag_shader));

  const MGPUShaderStageCreateInfo shader_stages[2] {
    {
      .stage = MGPU_SHADER_STAGE_VERTEX,
      .module = m_mgpu_vert_shader,
      .entrypoint = "main"
    },
    {
      .stage = MGPU_SHADER_STAGE_FRAGMENT,
      .module = m_mgpu_frag_shader,
      .entrypoint = "main"
    }
  };
  const MGPUShaderProgramCreateInfo shader_program_create_info{
    .shader_stage_count = 2u,
    .shader_stages = shader_stages,
    .resource_set_count = 1u,
    .resource_set_layouts = &m_mgpu_resource_set_layout
  };
  MGPU_CHECK(mgpuDeviceCreateShaderProgram(m_mgpu_device, &shader_program_create_info, &m_mgpu_shader_program));

  const MGPUVertexBinding vertex_input_binding{
    .binding = 0u,
    .stride = sizeof(float) * 6,
    .input_rate = MGPU_VERTEX_INPUT_RATE_VERTEX
  };
  const MGPUVertexAttribute vertex_input_attributes[]{
    {
      .location = 0u,
      .binding = 0u,
      .format = MGPU_VERTEX_FORMAT_STUB_XYZ323232,
      .offset = 0u
    },
    {
      .location = 1u,
      .binding = 0u,
      .format = MGPU_VERTEX_FORMAT_STUB_XYZ323232,
      .offset = sizeof(float) * 3
    }
  };
  const MGPUVertexInputStateCreateInfo vertex_input_state_create_info{
    .binding_count = 1u,
    .bindings = &vertex_input_binding,
    .attribute_count = sizeof(vertex_input_attributes) / sizeof(MGPUVertexAttribute),
    .attributes = vertex_input_attributes
  };
  MGPU_CHECK(mgpuDeviceCreateVertexInputState(m_mgpu_device, &vertex_input_state_create_info, &m_mgpu_vertex_input_state));

  const MGPURasterizerStateCreateInfo rasterizer_state_create_info{
    .depth_clamp_enable = false,
    .rasterizer_discard_enable = false,
    .polygon_mode = MGPU_POLYGON_MODE_FILL,
    .cull_mode = MGPU_CULL_MODE_BACK,
    .front_face = MGPU_FRONT_FACE_COUNTER_CLOCKWISE,
    .depth_bias_enable = false,
    .depth_bias_constant_factor = 0.f,
    .depth_bias_clamp = 0.f,
    .depth_bias_slope_factor = 0.f,
    .line_width = 1.f
  };
  MGPU_CHECK(mgpuDeviceCreateRasterizerState(m_mgpu_device, &rasterizer_state_create_info, &m_mgpu_rasterizer_state));

  const MGPUDepthStencilStateCreateInfo depth_stencil_state_create_info{
    .depth_test_enable = true,
    .depth_write_enable = true,
    .depth_compare_op = MGPU_COMPARE_OP_LESS,
    .stencil_test_enable = false
  };
  MGPU_CHECK(mgpuDeviceCreateDepthStencilState(m_mgpu_device, &depth_stencil_state_create_info, &m_mgpu_depth_stencil_state));

  MGPU_CHECK(mgpuDeviceCreateCommandList(m_mgpu_device, &m_mgpu_cmd_list));

  MainLoop();
}

Application::~Application() {
  mgpuCommandListDestroy(m_mgpu_cmd_list);
  mgpuDepthStencilStateDestroy(m_mgpu_depth_stencil_state);
  mgpuRasterizerStateDestroy(m_mgpu_rasterizer_state);
  mgpuVertexInputStateDestroy(m_mgpu_vertex_input_state);
  mgpuShaderProgramDestroy(m_mgpu_shader_program);
  mgpuShaderModuleDestroy(m_mgpu_frag_shader);
  mgpuShaderModuleDestroy(m_mgpu_vert_shader);
  mgpuResourceSetDestroy(m_mgpu_resource_set);
  mgpuResourceSetLayoutDestroy(m_mgpu_resource_set_layout);
  mgpuBufferDestroy(m_mgpu_ubo);
  mgpuBufferDestroy(m_mgpu_ibo);
  mgpuBufferDestroy(m_mgpu_vbo);
  mgpuTextureViewDestroy(m_mgpu_depth_texture_view);
  mgpuTextureDestroy(m_mgpu_depth_texture);
  for(MGPUTextureView texture_view : m_mgpu_swap_chain_texture_views) mgpuTextureViewDestroy(texture_view);
  mgpuSwapChainDestroy(m_mgpu_swap_chain);
  mgpuDeviceDestroy(m_mgpu_device);
  mgpuSurfaceDestroy(m_mgpu_surface);
  mgpuInstanceDestroy(m_mgpu_instance);
  SDL_DestroyWindow(m_sdl_window);
}

void Application::MainLoop() {
  MGPUQueue mgpu_queue = mgpuDeviceGetQueue(m_mgpu_device, MGPU_QUEUE_TYPE_GRAPHICS_COMPUTE);

  SDL_Event event{};

  while(true) {
    u32 texture_index{};
    MGPU_CHECK(mgpuSwapChainAcquireNextTexture(m_mgpu_swap_chain, &texture_index));

    atom::Matrix4 model_matrix = atom::Matrix4::RotationY(m_y_rotation);
    atom::Matrix4 view_matrix = atom::Matrix4::Translation(0.f, 0.f, -5.f);
    atom::Matrix4 modelview_matrix = view_matrix * model_matrix;
    MGPU_CHECK(mgpuQueueBufferUpload(mgpu_queue, m_mgpu_ubo,  64u, sizeof(modelview_matrix), &modelview_matrix));
    MGPU_CHECK(mgpuQueueBufferUpload(mgpu_queue, m_mgpu_ubo, 128u, sizeof(view_matrix), &view_matrix));
    m_y_rotation += 0.02f;

    MGPU_CHECK(mgpuCommandListClear(m_mgpu_cmd_list));

    const MGPURenderPassColorAttachment render_pass_color_attachments[1] {
      {
        .texture_view = m_mgpu_swap_chain_texture_views[texture_index],
        .load_op = MGPU_LOAD_OP_CLEAR,
        .store_op = MGPU_STORE_OP_STORE,
        .clear_color = {.r = 0.3f, .g = 0.f, .b = 0.9f, .a = 1.f}
      }
    };
     const MGPURenderPassDepthStencilAttachment render_pass_depth_stencil_attachment{
       .texture_view = m_mgpu_depth_texture_view,
       .depth_load_op = MGPU_LOAD_OP_CLEAR,
       .depth_store_op = MGPU_STORE_OP_STORE,
       .stencil_load_op = MGPU_LOAD_OP_DONT_CARE,
       .stencil_store_op = MGPU_STORE_OP_DONT_CARE,
       .clear_depth = 1.f,
       .clear_stencil = 0u
     };
    const MGPURenderPassBeginInfo render_pass_info{
      .color_attachment_count = 1u,
      .color_attachments = render_pass_color_attachments,
      .depth_stencil_attachment = &render_pass_depth_stencil_attachment
    };
    MGPURenderCommandEncoder render_cmd_encoder = mgpuCommandListCmdBeginRenderPass(m_mgpu_cmd_list, &render_pass_info);
    mgpuRenderCommandEncoderCmdUseShaderProgram(render_cmd_encoder, m_mgpu_shader_program);
    mgpuRenderCommandEncoderCmdUseRasterizerState(render_cmd_encoder, m_mgpu_rasterizer_state);
    mgpuRenderCommandEncoderCmdUseDepthStencilState(render_cmd_encoder, m_mgpu_depth_stencil_state);
    mgpuRenderCommandEncoderCmdUseVertexInputState(render_cmd_encoder, m_mgpu_vertex_input_state);
    mgpuRenderCommandEncoderCmdBindVertexBuffer(render_cmd_encoder, 0u, m_mgpu_vbo, 0u);
    mgpuRenderCommandEncoderCmdBindIndexBuffer(render_cmd_encoder, m_mgpu_ibo, 0u, MGPU_INDEX_FORMAT_U16);
    mgpuRenderCommandEncoderCmdBindResourceSet(render_cmd_encoder, 0u, m_mgpu_resource_set);
    mgpuRenderCommandEncoderCmdDrawIndexed(render_cmd_encoder, 36u, 1u, 0u, 0u, 0u);
    mgpuRenderCommandEncoderClose(render_cmd_encoder);

    MGPU_CHECK(mgpuQueueSubmitCommandList(mgpu_queue, m_mgpu_cmd_list));
    MGPU_CHECK(mgpuSwapChainPresent(m_mgpu_swap_chain));

    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
        return;
      }
    }
  }
}
