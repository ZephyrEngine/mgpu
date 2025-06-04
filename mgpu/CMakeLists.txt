
# TODO: allow disabling specific backends during the build

set(SOURCES
  src/backend/command_list/render_command_encoder.cpp
  src/backend/vulkan/lib/vulkan_instance.cpp
  src/backend/vulkan/lib/vulkan_physical_device.cpp
  src/backend/vulkan/pipeline_state/color_blend_state.cpp
  src/backend/vulkan/pipeline_state/depth_stencil_state.cpp
  src/backend/vulkan/pipeline_state/input_assembly_state.cpp
  src/backend/vulkan/pipeline_state/rasterizer_state.cpp
  src/backend/vulkan/pipeline_state/shader_module.cpp
  src/backend/vulkan/pipeline_state/shader_program.cpp
  src/backend/vulkan/pipeline_state/vertex_input_state.cpp
  src/backend/vulkan/buffer.cpp
  src/backend/vulkan/deleter_queue.cpp
  src/backend/vulkan/device.cpp
  src/backend/vulkan/graphics_pipeline_cache.cpp
  src/backend/vulkan/instance.cpp
  src/backend/vulkan/physical_device.cpp
  src/backend/vulkan/queue.cpp
  src/backend/vulkan/resource_set_layout.cpp
  src/backend/vulkan/resource_set.cpp
  src/backend/vulkan/render_pass_cache.cpp
  src/backend/vulkan/sampler.cpp
  src/backend/vulkan/surface.cpp
  src/backend/vulkan/swap_chain.cpp
  src/backend/vulkan/texture.cpp
  src/backend/vulkan/texture_view.cpp
  src/backend/device.cpp
  src/frontend/buffer.cpp
  src/frontend/command_list.cpp
  src/frontend/destroy.cpp
  src/frontend/device.cpp
  src/frontend/instance.cpp
  src/frontend/physical_device.cpp
  src/frontend/queue.cpp
  src/frontend/render_command_encoder.cpp
  src/frontend/result.cpp
  src/frontend/swap_chain.cpp
  src/frontend/texture.cpp
)

set(HEADERS
  src/backend/vulkan/lib/vulkan_instance.hpp
  src/backend/vulkan/lib/vulkan_physical_device.hpp
  src/backend/vulkan/lib/vulkan_result.hpp
  src/backend/vulkan/platform/physical_device.hpp
  src/backend/vulkan/platform/surface.hpp
  src/backend/vulkan/pipeline_state/color_blend_state.hpp
  src/backend/vulkan/pipeline_state/depth_stencil_state.hpp
  src/backend/vulkan/pipeline_state/input_assembly_state.hpp
  src/backend/vulkan/pipeline_state/rasterizer_state.hpp
  src/backend/vulkan/pipeline_state/shader_module.hpp
  src/backend/vulkan/pipeline_state/shader_program.hpp
  src/backend/vulkan/pipeline_state/vertex_input_state.hpp
  src/backend/vulkan/buffer.hpp
  src/backend/vulkan/deleter_queue.hpp
  src/backend/vulkan/device.hpp
  src/backend/vulkan/graphics_pipeline_cache.hpp
  src/backend/vulkan/instance.hpp
  src/backend/vulkan/physical_device.hpp
  src/backend/vulkan/queue.hpp
  src/backend/vulkan/render_pass_cache.hpp
  src/backend/vulkan/resource_set_layout.hpp
  src/backend/vulkan/resource_set.hpp
  src/backend/vulkan/sampler.hpp
  src/backend/vulkan/surface.hpp
  src/backend/vulkan/swap_chain.hpp
  src/backend/vulkan/texture.hpp
  src/backend/vulkan/texture_view.hpp
  src/backend/command_list/command_list.hpp
  src/backend/command_list/commands.hpp
  src/backend/command_list/render_command_encoder.hpp
  src/backend/pipeline_state/color_blend_state.hpp
  src/backend/pipeline_state/depth_stencil_state.hpp
  src/backend/pipeline_state/input_assembly_state.hpp
  src/backend/pipeline_state/rasterizer_state.hpp
  src/backend/pipeline_state/shader_module.hpp
  src/backend/pipeline_state/shader_program.hpp
  src/backend/pipeline_state/vertex_input_state.hpp
  src/backend/buffer.hpp
  src/backend/device.hpp
  src/backend/instance.hpp
  src/backend/physical_device.hpp
  src/backend/queue.hpp
  src/backend/resource_set_layout.hpp
  src/backend/resource_set.hpp
  src/backend/sampler.hpp
  src/backend/surface.hpp
  src/backend/swap_chain.hpp
  src/backend/texture.hpp
  src/backend/texture_view.hpp
  src/frontend/validation/buffer.hpp
  src/frontend/validation/sampler.hpp
  src/frontend/validation/shader_program.hpp
  src/frontend/validation/texture.hpp
  src/frontend/validation/texture_view.hpp
  src/common/bump_allocator.hpp
  src/common/limits.hpp
  src/common/result.hpp
  src/common/texture.hpp
)

set(HEADERS_PUBLIC
  include/mgpu/mgpu.h
)

if(WIN32)
  list(APPEND SOURCES
    src/backend/vulkan/platform/win32/physical_device.cpp
    src/backend/vulkan/platform/win32/surface.cpp
  )
elseif(APPLE)
  enable_language(OBJCXX)
  list(APPEND SOURCES
    src/backend/vulkan/platform/metal/physical_device.cpp
    src/backend/vulkan/platform/metal/surface.mm
  )
elseif(UNIX)
  # TODO: how to deal with X11 and Wayland at the same time?
  list(APPEND SOURCES
    src/backend/vulkan/platform/wayland_x11/physical_device.cpp
    src/backend/vulkan/platform/wayland_x11/surface.cpp
  )
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

find_package(Vulkan REQUIRED)

add_library(mgpu ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})

target_include_directories(mgpu PUBLIC include)
target_include_directories(mgpu PRIVATE src)
target_link_libraries(mgpu PUBLIC mgpu-cxx-opts Vulkan::Vulkan)
target_link_libraries(mgpu PRIVATE atom-common fmt VulkanMemoryAllocator)