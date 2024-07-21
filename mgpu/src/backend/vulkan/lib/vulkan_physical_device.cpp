
#include "vulkan_physical_device.hpp"

namespace mgpu::vulkan {

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice vk_physical_device)
    : m_vk_physical_device{vk_physical_device} {
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

const VkPhysicalDeviceProperties& VulkanPhysicalDevice::GetProperties() const {
  return m_vk_device_properties;
}

std::span<const VkExtensionProperties> VulkanPhysicalDevice::EnumerateDeviceExtensions() const {
  return m_vk_available_device_extensions;
}

std::span<const VkLayerProperties> VulkanPhysicalDevice::EnumerateDeviceLayers() const {
  return m_vk_available_device_layers;
}

std::span<const VkQueueFamilyProperties> VulkanPhysicalDevice::EnumerateQueueFamilies() const {
  return m_vk_queue_family_properties;
}

bool VulkanPhysicalDevice::QueryDeviceExtensionSupport(const char* extension_name) const {
  const auto predicate = [&](const VkExtensionProperties& extension) {
    return std::strcmp(extension.extensionName, extension_name) == 0;
  };
  return std::ranges::find_if(m_vk_available_device_extensions, predicate) != m_vk_available_device_extensions.end();
}

bool VulkanPhysicalDevice::QueryDeviceLayerSupport(const char* layer_name) const {
  const auto predicate = [&](const VkLayerProperties& layer) {
    return std::strcmp(layer.layerName, layer_name) == 0;
  };
  return std::ranges::find_if(m_vk_available_device_layers, predicate) != m_vk_available_device_layers.end();
}

[[nodiscard]] Result<VkDevice> VulkanPhysicalDevice::CreateLogicalDevice(
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

}  // namespace mgpu::vulkan
