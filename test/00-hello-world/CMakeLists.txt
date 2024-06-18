
set(SOURCES
  src/main.cpp
)

set(HEADERS
)

find_package(SDL2 REQUIRED)

add_executable(test-hello-world ${SOURCES} ${HEADERS})

target_include_directories(test-hello-world PRIVATE src)
target_link_libraries(test-hello-world PRIVATE mgpu mgpu-cxx-opts atom-common SDL2::SDL2)