
# TODO: allow disabling specific backends during the build

set(SOURCES
  src/backend/vulkan/lib/vulkan_instance.cpp
  src/backend/vulkan/lib/vulkan_physical_device.cpp
  src/backend/vulkan/instance.cpp
  src/backend/vulkan/physical_device.cpp
  src/frontend/instance.cpp
  src/frontend/physical_device.cpp
  src/frontend/result.cpp
)

set(HEADERS
  src/backend/vulkan/lib/vulkan_instance.hpp
  src/backend/vulkan/lib/vulkan_physical_device.hpp
  src/backend/vulkan/lib/vulkan_result.hpp
  src/backend/vulkan/instance.hpp
  src/backend/vulkan/physical_device.hpp
  src/backend/buffer.hpp
  src/backend/device.hpp
  src/backend/instance.hpp
  src/backend/physical_device.hpp
  src/common/result.hpp
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