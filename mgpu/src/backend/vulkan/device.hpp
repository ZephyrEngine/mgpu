
#pragma once

#include <atom/integer.hpp>
#include <optional>
#include <vulkan/vulkan.h>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/device.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device final : public DeviceBase {
  public:
   ~Device() override;

    static Result<DeviceBase*> Create(VulkanPhysicalDevice& vk_physical_device);

  private:
    struct QueueFamilyIndices {
      std::optional<u32> graphics_and_compute{};
      std::optional<u32> dedicated_compute{};
    };

    explicit Device(VkDevice vk_device);

    static QueueFamilyIndices SelectQueueFamilies(VulkanPhysicalDevice& vk_physical_device);

    VkDevice m_vk_device{};
};

}  // namespace mgpu::vulkan
