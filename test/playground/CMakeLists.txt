
set(SOURCES
  src/main.cpp
)

set(HEADERS
  src/shader/triangle.frag.h
  src/shader/triangle.vert.h
)

find_package(SDL2 REQUIRED)

add_executable(test-playground ${SOURCES} ${HEADERS})

target_include_directories(test-playground PRIVATE src)
target_link_libraries(test-playground PRIVATE mgpu mgpu-cxx-opts atom-common atom-logger SDL2::SDL2 VulkanMemoryAllocator)