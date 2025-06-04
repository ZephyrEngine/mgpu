
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/platform/surface.hpp"

namespace mgpu::vulkan {

const char* PlatformGetSurfaceInstanceExtension() {
  return "VK_KHR_wayland_surface";
}

Result<VkSurfaceKHR> PlatformCreateSurface(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  const VkWaylandSurfaceCreateInfoKHR vk_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .display = create_info.wayland.display,
    .surface = create_info.wayland.surface
  };

  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateWaylandSurfaceKHR(vk_instance, &vk_surface_create_info, nullptr, &vk_surface));
  return vk_surface;
}

}  // namespace mgpu::vulkan
