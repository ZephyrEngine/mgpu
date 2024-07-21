
#pragma once

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <algorithm>
#include <cstring>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/result.hpp"
#include "vulkan_result.hpp"

namespace mgpu::vulkan {

class VulkanPhysicalDevice {
  public:
    explicit VulkanPhysicalDevice(VkPhysicalDevice vk_physical_device);

    VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
    VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;

    [[nodiscard]] const VkPhysicalDeviceProperties& GetProperties() const;
    [[nodiscard]] std::span<const VkExtensionProperties> EnumerateDeviceExtensions() const;
    [[nodiscard]] std::span<const VkLayerProperties> EnumerateDeviceLayers() const;
    [[nodiscard]] std::span<const VkQueueFamilyProperties> EnumerateQueueFamilies() const;
    [[nodiscard]] bool QueryDeviceExtensionSupport(const char* extension_name) const;
    [[nodiscard]] bool QueryDeviceLayerSupport(const char* layer_name) const;

    [[nodiscard]] Result<VkDevice> CreateLogicalDevice(
      std::span<const VkDeviceQueueCreateInfo> queue_create_infos,
      std::span<const char* const> required_device_extensions,
      std::span<const char* const> required_device_layers
    ) const;

    [[nodiscard]] VkPhysicalDevice Handle() const {
      return m_vk_physical_device;
    }

  private:
    VkPhysicalDevice m_vk_physical_device{};
    VkPhysicalDeviceProperties m_vk_device_properties{};
    std::vector<VkExtensionProperties> m_vk_available_device_extensions{};
    std::vector<VkLayerProperties> m_vk_available_device_layers{};
    std::vector<VkQueueFamilyProperties> m_vk_queue_family_properties{};
};

}  // namespace mgpu::vulkan
