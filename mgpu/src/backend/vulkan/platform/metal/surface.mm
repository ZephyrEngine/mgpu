
/**
 * This fixes a compiler error when including {fmt} from an Objective-C++ file.
 * For some reason when compiling Objective-C++ __EXCEPTIONS appears to be defined even when exceptions are disabled.
 * This will cause {fmt} to try use throw, which will error out.
 * So we explicitly tell {fmt} to not use exceptions before including it.
 */
#define FMT_EXCEPTIONS 0

#include <vulkan/vulkan.h>       // Must be included before vulkan_metal.h
#include <vulkan/vulkan_metal.h>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/platform/surface.hpp"

namespace mgpu::vulkan {

std::vector<const char*> PlatformGetSurfaceInstanceExtensions() {
  return {VK_EXT_METAL_SURFACE_EXTENSION_NAME};
}

Result<VkSurfaceKHR> PlatformCreateSurface(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  const VkMetalSurfaceCreateInfoEXT vk_metal_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .pLayer = create_info.metal.metal_layer
  };

  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateMetalSurfaceEXT(vk_instance, &vk_metal_surface_create_info, nullptr, &vk_surface));
  return vk_surface;
}

}  // namespace mgpu::vulkan
