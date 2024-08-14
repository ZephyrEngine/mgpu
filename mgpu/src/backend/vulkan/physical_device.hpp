
#pragma once

#include <atom/integer.hpp>
#include <optional>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/physical_device.hpp"

namespace mgpu::vulkan {

class PhysicalDevice final : public PhysicalDeviceBase {
  public:
    struct QueueFamilyIndices {
      std::optional<u32> graphics_and_compute{};
      std::optional<u32> dedicated_compute{};
    };

    explicit PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device, const QueueFamilyIndices& queue_family_indices);

    Result<MGPUSurfaceCapabilities> GetSurfaceCapabilities(mgpu::SurfaceBase* surface) override;
    Result<std::vector<MGPUSurfaceFormat>> EnumerateSurfaceFormats(mgpu::SurfaceBase* surface) override;
    Result<std::vector<MGPUPresentMode>> EnumerateSurfacePresentModes(mgpu::SurfaceBase* surface) override;
    Result<DeviceBase*> CreateDevice() override;

  private:
    //void PopulatePhysicalDeviceInfo();
    static MGPUPhysicalDeviceInfo GetInfo(VulkanPhysicalDevice& vk_physical_device);
    static MGPUPhysicalDeviceLimits GetLimits(const VkPhysicalDeviceLimits& vk_device_limits);

    VkInstance m_vk_instance{};
    VulkanPhysicalDevice& m_vk_physical_device;
    QueueFamilyIndices m_queue_family_indices;
};

}  // namespace mgpu::vulkan
