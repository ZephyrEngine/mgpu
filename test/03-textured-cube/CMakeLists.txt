
set(SOURCES
  src/application.cpp
  src/main.cpp
)

set(HEADERS
  src/shader/cube.frag.h
  src/shader/cube.vert.h
  src/application.hpp
)

set(LIBRARIES
  mgpu mgpu-cxx-opts atom-common atom-math SDL2::SDL2
)

if(APPLE)
  # TODO: write SDL2 "plugin" for mgpu to simplify this mess
  enable_language(OBJCXX)
  list(APPEND SOURCES
    src/tmp_cocoa.mm
  )
  list(APPEND LIBRARIES
    "-framework Cocoa"
    "-framework QuartzCore"
  )
endif()

find_package(SDL2 REQUIRED)

add_executable(test-textured-cube ${SOURCES} ${HEADERS})

target_include_directories(test-textured-cube PRIVATE src)
target_link_libraries(test-textured-cube PRIVATE ${LIBRARIES})