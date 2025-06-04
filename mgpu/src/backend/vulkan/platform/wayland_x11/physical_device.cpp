
#include <vulkan/vulkan.h>

#include "backend/vulkan/platform/physical_device.hpp"

namespace mgpu::vulkan {

bool PlatformQueryPresentationSupport(VkPhysicalDevice vk_physical_device, uint32_t queue_family_index) {
  // TODO: if possible actually query the presentation support. Sadly this requires knowing the wl_display,
  // which isn´t available to us at this point.
  return true;
}

}  // namespace mgpu::vulkan
