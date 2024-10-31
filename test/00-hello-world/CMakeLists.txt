
set(SOURCES
  src/main.cpp
)

set(HEADERS
  src/shader/triangle.frag.h
  src/shader/triangle.vert.h
)

set(LIBRARIES
  mgpu mgpu-cxx-opts atom-common SDL2::SDL2
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

add_executable(test-hello-world ${SOURCES} ${HEADERS})

target_include_directories(test-hello-world PRIVATE src)
target_link_libraries(test-hello-world PRIVATE ${LIBRARIES})