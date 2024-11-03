
#include "backend/vulkan/platform/physical_device.hpp"

namespace mgpu::vulkan {

bool PlatformQueryPresentationSupport(VkPhysicalDevice vk_physical_device, uint32_t queue_family_index) {
  (void)vk_physical_device;
  (void)queue_family_index;
  return true;
}

}  // namespace mgpu::vulkan
