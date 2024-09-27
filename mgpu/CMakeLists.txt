
# TODO: allow disabling specific backends during the build

set(SOURCES
  src/backend/vulkan/lib/vulkan_instance.cpp
  src/backend/vulkan/lib/vulkan_physical_device.cpp
  src/backend/vulkan/buffer.cpp
  src/backend/vulkan/command_queue.cpp
  src/backend/vulkan/deleter_queue.cpp
  src/backend/vulkan/device.cpp
  src/backend/vulkan/instance.cpp
  src/backend/vulkan/physical_device.cpp
  src/backend/vulkan/render_target.cpp
  src/backend/vulkan/surface.cpp
  src/backend/vulkan/swap_chain.cpp
  src/backend/vulkan/texture.cpp
  src/backend/vulkan/texture_view.cpp
  src/frontend/buffer.cpp
  src/frontend/command_list.cpp
  src/frontend/device.cpp
  src/frontend/instance.cpp
  src/frontend/physical_device.cpp
  src/frontend/render_target.cpp
  src/frontend/result.cpp
  src/frontend/surface.cpp
  src/frontend/swap_chain.cpp
  src/frontend/texture.cpp
  src/frontend/texture_view.cpp
)

set(HEADERS
  src/backend/vulkan/lib/vulkan_instance.hpp
  src/backend/vulkan/lib/vulkan_physical_device.hpp
  src/backend/vulkan/lib/vulkan_result.hpp
  src/backend/vulkan/platform/physical_device.hpp
  src/backend/vulkan/platform/surface.hpp
  src/backend/vulkan/buffer.hpp
  src/backend/vulkan/command_queue.hpp
  src/backend/vulkan/deleter_queue.hpp
  src/backend/vulkan/device.hpp
  src/backend/vulkan/instance.hpp
  src/backend/vulkan/physical_device.hpp
  src/backend/vulkan/render_target.hpp
  src/backend/vulkan/surface.hpp
  src/backend/vulkan/swap_chain.hpp
  src/backend/vulkan/texture.hpp
  src/backend/vulkan/texture_view.hpp
  src/backend/buffer.hpp
  src/backend/command_list.hpp
  src/backend/device.hpp
  src/backend/instance.hpp
  src/backend/physical_device.hpp
  src/backend/render_target.hpp
  src/backend/surface.hpp
  src/backend/swap_chain.hpp
  src/backend/texture.hpp
  src/backend/texture_view.hpp
  src/frontend/validation/buffer.hpp
  src/frontend/validation/texture.hpp
  src/frontend/validation/texture_view.hpp
  src/common/bump_allocator.hpp
  src/common/result.hpp
  src/common/texture.hpp
)

set(HEADERS_PUBLIC
  include/mgpu/mgpu.h
        src/frontend/render_target.cpp
)

if(WIN32)
  list(APPEND SOURCES
    src/backend/vulkan/platform/win32/physical_device.cpp
    src/backend/vulkan/platform/win32/surface.cpp
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