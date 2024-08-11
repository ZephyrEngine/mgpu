
#pragma once

#include <vulkan/vulkan.h>

namespace mgpu::vulkan {

bool PlatformQueryPresentationSupport(VkPhysicalDevice vk_physical_device, uint32_t queue_family_index);

}  // namespace mgpu::vulkan
