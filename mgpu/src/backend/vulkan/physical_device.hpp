
#pragma once

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/physical_device.hpp"

namespace mgpu::vulkan {

class PhysicalDevice final : public PhysicalDeviceBase {
  public:
    explicit PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device);

    MGPUResult GetInfo(MGPUPhysicalDeviceInfo& physical_device_info) override;
    Result<DeviceBase*> CreateDevice() override;

  private:
    void PopulatePhysicalDeviceInfo();

    VkInstance m_vk_instance{};
    VulkanPhysicalDevice& m_vk_physical_device;
    MGPUPhysicalDeviceInfo m_physical_device_info{};
};

}  // namespace mgpu::vulkan
