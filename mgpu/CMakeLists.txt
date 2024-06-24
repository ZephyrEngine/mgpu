
# TODO: split this up into multiple libraries in the future (i.e. one core library and one library per backend)

# TODO: remove dependency on SDL2 if possible

set(SOURCES
  src/backend/opengl/buffer.cpp
  src/backend/opengl/render_device_backend.cpp
  src/backend/vulkan/utility/vulkan_command_buffer.cpp
  src/backend/vulkan/utility/vulkan_command_pool.cpp
  src/backend/vulkan/utility/vulkan_fence.cpp
  src/backend/vulkan/utility/vulkan_instance.cpp
  src/backend/vulkan/render_device_backend.cpp
  src/frontend/render_device.cpp
  src/interface.cpp
)

set(HEADERS
  src/backend/opengl/buffer.hpp
  src/backend/opengl/render_device_backend.hpp
  src/backend/vulkan/utility/vulkan_command_buffer.hpp
  src/backend/vulkan/utility/vulkan_command_pool.hpp
  src/backend/vulkan/utility/vulkan_fence.hpp
  src/backend/vulkan/utility/vulkan_instance.hpp
  src/backend/vulkan/utility/vulkan_physical_device.hpp
  src/backend/vulkan/render_device_backend.hpp
  src/backend/render_device_backend_base.hpp
  src/frontend/buffer.hpp
  src/frontend/render_device.hpp
  src/common/result.hpp
)

set(HEADERS_PUBLIC
  include/mgpu/mgpu.h
)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(mgpu ${SOURCES} ${HEADERS} ${HEADERS_PUBLIC})

target_include_directories(mgpu PUBLIC include)
target_include_directories(mgpu PRIVATE src)
target_link_libraries(mgpu PUBLIC mgpu-cxx-opts SDL2::SDL2 Vulkan::Vulkan OpenGL::GL GLEW::GLEW)
target_link_libraries(mgpu PRIVATE atom-common fmt)