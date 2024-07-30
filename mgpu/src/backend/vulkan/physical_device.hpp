
#pragma once

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/physical_device.hpp"

namespace mgpu::vulkan {

class PhysicalDevice final : public PhysicalDeviceBase {
  public:
    explicit PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device);

    Result<DeviceBase*> CreateDevice() override;

  private:
    //void PopulatePhysicalDeviceInfo();
    static MGPUPhysicalDeviceInfo GetInfo(VulkanPhysicalDevice& vk_physical_device);
    static MGPUPhysicalDeviceLimits GetLimits(const VkPhysicalDeviceLimits& vk_device_limits);

    VkInstance m_vk_instance{};
    VulkanPhysicalDevice& m_vk_physical_device;
};

}  // namespace mgpu::vulkan
