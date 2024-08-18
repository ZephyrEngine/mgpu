
#include <windows.h>             // Must be included before vulkan_win32.h
#include <vulkan/vulkan.h>       // Must be included before vulkan_win32.h
#include <vulkan/vulkan_win32.h>

#include "backend/vulkan/platform/physical_device.hpp"

namespace mgpu::vulkan {

bool PlatformQueryPresentationSupport(VkPhysicalDevice vk_physical_device, uint32_t queue_family_index) {
  return vkGetPhysicalDeviceWin32PresentationSupportKHR(vk_physical_device, queue_family_index);
}

}  // namespace mgpu::vulkan
