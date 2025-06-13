
#pragma once

#include <mgpu/mgpu.h>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/result.hpp"

namespace mgpu::vulkan {

std::vector<const char*> PlatformGetSurfaceInstanceExtensions();
Result<VkSurfaceKHR> PlatformCreateSurface(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info);

}  // namespace mgpu::vulkan
