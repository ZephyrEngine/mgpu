
#include <windows.h>             // Must be included before vulkan_win32.h
#include <vulkan/vulkan.h>       // Must be included before vulkan_win32.h
#include <vulkan/vulkan_win32.h>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/platform/surface.hpp"

namespace mgpu::vulkan {

const char* PlatformGetSurfaceInstanceExtension() {
  return "VK_KHR_win32_surface";
}

Result<VkSurfaceKHR> PlatformCreateSurface(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  const VkWin32SurfaceCreateInfoKHR vk_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .hinstance = create_info.win32.hinstance,
    .hwnd = create_info.win32.hwnd
  };

  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateWin32SurfaceKHR(vk_instance, &vk_surface_create_info, nullptr, &vk_surface));
  return vk_surface;
}

}  // namespace mgpu::vulkan
