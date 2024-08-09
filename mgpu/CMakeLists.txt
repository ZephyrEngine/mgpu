
# TODO: allow disabling specific backends during the build

set(SOURCES
  src/backend/vulkan/lib/vulkan_instance.cpp
  src/backend/vulkan/lib/vulkan_physical_device.cpp
  src/backend/vulkan/buffer.cpp
  src/backend/vulkan/deleter_queue.cpp
  src/backend/vulkan/device.cpp
  src/backend/vulkan/instance.cpp
  src/backend/vulkan/physical_device.cpp
  src/backend/vulkan/surface.cpp
  src/backend/vulkan/texture.cpp
  src/backend/vulkan/texture_view.cpp
  src/frontend/buffer.cpp
  src/frontend/device.cpp
  src/frontend/instance.cpp
  src/frontend/physical_device.cpp
  src/frontend/result.cpp
  src/frontend/surface.cpp
  src/frontend/texture.cpp
  src/frontend/texture_view.cpp
)

set(HEADERS
  src/backend/vulkan/lib/vulkan_instance.hpp
  src/backend/vulkan/lib/vulkan_physical_device.hpp
  src/backend/vulkan/lib/vulkan_result.hpp
  src/backend/vulkan/buffer.hpp
  src/backend/vulkan/deleter_queue.hpp
  src/backend/vulkan/device.hpp
  src/backend/vulkan/instance.hpp
  src/backend/vulkan/physical_device.hpp
  src/backend/vulkan/surface.hpp
  src/backend/vulkan/texture.hpp
  src/backend/vulkan/texture_view.hpp
  src/backend/buffer.hpp
  src/backend/device.hpp
  src/backend/instance.hpp
  src/backend/physical_device.hpp
  src/backend/surface.hpp
  src/backend/texture.hpp
  src/backend/texture_view.hpp
  src/frontend/validation/buffer.hpp
  src/frontend/validation/texture.hpp
  src/frontend/validation/texture_view.hpp
  src/common/result.hpp
  src/common/texture.hpp
)

set(HEADERS_PUBLIC
  include/mgpu/mgpu.h
)

find_package(Vulkan REQUIRED)

add_library(mgpu ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})

target_include_directories(mgpu PUBLIC include)
target_include_directories(mgpu PRIVATE src)
target_link_libraries(mgpu PUBLIC mgpu-cxx-opts Vulkan::Vulkan)
target_link_libraries(mgpu PRIVATE atom-common fmt VulkanMemoryAllocator)