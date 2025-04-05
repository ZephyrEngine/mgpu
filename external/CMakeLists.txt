
if(NOT TARGET atom-common)
  add_subdirectory(atom)
endif()

# TODO: VMA wants this. Maybe only add VMA if we build with the Vulkan backend?
find_package(Vulkan REQUIRED)
add_subdirectory(VulkanMemoryAllocator)