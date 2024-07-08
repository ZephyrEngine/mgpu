
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

namespace mgpu {

class VulkanPhysicalDevice {
  public:
    explicit VulkanPhysicalDevice(VkPhysicalDevice vk_physical_device) : m_vk_physical_device{vk_physical_device} {
      vkGetPhysicalDeviceProperties(m_vk_physical_device, &m_vk_device_properties);

      u32 extension_count;
      vkEnumerateDeviceExtensionProperties(m_vk_physical_device, nullptr, &extension_count, nullptr);
      m_vk_available_device_extensions.resize(extension_count);
      vkEnumerateDeviceExtensionProperties(m_vk_physical_device, nullptr, &extension_count, m_vk_available_device_extensions.data());

      u32 layer_count;
      vkEnumerateDeviceLayerProperties(m_vk_physical_device, &layer_count, nullptr);
      m_vk_available_device_layers.resize(layer_count);
      vkEnumerateDeviceLayerProperties(m_vk_physical_device, &layer_count, m_vk_available_device_layers.data());

      u32 queue_family_count;
      vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &queue_family_count, nullptr);
      m_vk_queue_family_properties.resize(queue_family_count);
      vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &queue_family_count, m_vk_queue_family_properties.data());
    }

    VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
    VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;

    [[nodiscard]] VkPhysicalDevice Handle() const {
      return m_vk_physical_device;
    }

    [[nodiscard]] const VkPhysicalDeviceProperties& GetProperties() const {
      return m_vk_device_properties;
    }

    [[nodiscard]] std::span<const VkExtensionProperties> EnumerateDeviceExtensions() const {
      return m_vk_available_device_extensions;
    }

    [[nodiscard]] std::span<const VkLayerProperties> EnumerateDeviceLayers() const {
      return m_vk_available_device_layers;
    }

    [[nodiscard]] std::span<const VkQueueFamilyProperties> EnumerateQueueFamilies() const {
      return m_vk_queue_family_properties;
    }

    [[nodiscard]] bool QueryDeviceExtensionSupport(const char* extension_name) const {
      const auto predicate = [&](const VkExtensionProperties& extension) {
        return std::strcmp(extension.extensionName, extension_name) == 0;
      };
      return std::ranges::find_if(m_vk_available_device_extensions, predicate) != m_vk_available_device_extensions.end();
    }

    [[nodiscard]] bool QueryDeviceLayerSupport(const char* layer_name) const {
      const auto predicate = [&](const VkLayerProperties& layer) {
        return std::strcmp(layer.layerName, layer_name) == 0;
      };
      return std::ranges::find_if(m_vk_available_device_layers, predicate) != m_vk_available_device_layers.end();
    }

    [[nodiscard]] Result<VkDevice> CreateLogicalDevice(
      std::span<const VkDeviceQueueCreateInfo> queue_create_infos,
      std::span<const char* const> required_device_extensions,
      std::span<const char* const> required_device_layers
    ) const {
      const VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = (u32)queue_create_infos.size(),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = (u32)required_device_layers.size(),
        .ppEnabledLayerNames = required_device_layers.data(),
        .enabledExtensionCount = (u32)required_device_extensions.size(),
        .ppEnabledExtensionNames = required_device_extensions.data(),
        .pEnabledFeatures = nullptr
      };

      VkDevice vk_device{};

      if(VkResult vk_result = vkCreateDevice(m_vk_physical_device, &create_info, nullptr, &vk_device); vk_result != VK_SUCCESS) {
        vk_result_to_mgpu_result(vk_result);
      }
      return vk_device;
    }

  private:
    VkPhysicalDevice m_vk_physical_device{};
    VkPhysicalDeviceProperties m_vk_device_properties{};
    std::vector<VkExtensionProperties> m_vk_available_device_extensions{};
    std::vector<VkLayerProperties> m_vk_available_device_layers{};
    std::vector<VkQueueFamilyProperties> m_vk_queue_family_properties{};
};

}  // namespace mgpu