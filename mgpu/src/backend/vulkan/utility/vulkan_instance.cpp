
#include "vulkan_instance.hpp"

namespace mgpu {

std::vector<VkExtensionProperties> VulkanInstance::k_available_vk_instance_extensions = []() {
  u32 extension_count;
  std::vector<VkExtensionProperties> available_extensions{};

  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  available_extensions.resize(extension_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());
  return available_extensions;
}();

std::vector<VkLayerProperties> VulkanInstance::k_available_vk_instance_layers = []() {
  u32 layer_count;
  std::vector<VkLayerProperties> available_layers{};

  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  available_layers.resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  return available_layers;
}();

}  // namespace mgpu