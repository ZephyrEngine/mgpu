
#include "vulkan_instance.hpp"

namespace mgpu::vulkan {

VulkanInstance::VulkanInstance(VkInstance vk_instance)
    : m_vk_instance{vk_instance} {
  PopulatePhysicalDeviceList();
}

VulkanInstance::~VulkanInstance() {
  vkDestroyInstance(m_vk_instance, nullptr);
}

Result<std::unique_ptr<VulkanInstance>> VulkanInstance::Create(
  const VkApplicationInfo& app_info,
  std::vector<const char*> required_instance_extensions,
  std::vector<const char*> required_instance_layers,
  bool accept_vulkan_portability
) {
  for(const auto required_layer_name : required_instance_layers) {
    if(!QueryInstanceLayerSupport(required_layer_name)) {
      ATOM_PANIC("Could not find required Vulkan instance layer: {}", required_layer_name);
    }
  }

  VkInstanceCreateFlags instance_create_flags = 0;

  if(accept_vulkan_portability && QueryInstanceExtensionSupport(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
    required_instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    required_instance_extensions.push_back("VK_KHR_get_physical_device_properties2"); // required by VK_KHR_portability_subset
    instance_create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }

  for(const auto required_extension_name : required_instance_extensions) {
    if(!QueryInstanceExtensionSupport(required_extension_name)) {
      ATOM_PANIC("Could not find required Vulkan instance extension: {}", required_extension_name);
    }
  }

  const VkInstanceCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = instance_create_flags,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = (u32)required_instance_layers.size(),
    .ppEnabledLayerNames = required_instance_layers.data(),
    .enabledExtensionCount = (u32)required_instance_extensions.size(),
    .ppEnabledExtensionNames = required_instance_extensions.data()
  };

  VkInstance vk_instance{};

  if(VkResult vk_result = vkCreateInstance(&create_info, nullptr, &vk_instance); vk_result != VK_SUCCESS) {
    return vk_result_to_mgpu_result(vk_result);
  }
  return std::unique_ptr<VulkanInstance>{new VulkanInstance{vk_instance}};
}


bool VulkanInstance::QueryInstanceExtensionSupport(const char* extension_name) {
  const auto predicate = [&](const VkExtensionProperties& extension) {
    return std::strcmp(extension.extensionName, extension_name) == 0;
  };
  return std::ranges::find_if(k_available_vk_instance_extensions, predicate) != k_available_vk_instance_extensions.end();
}

bool VulkanInstance::QueryInstanceLayerSupport(const char* layer_name) {
  const auto predicate = [&](const VkLayerProperties& layer) {
    return std::strcmp(layer.layerName, layer_name) == 0;
  };
  return std::ranges::find_if(k_available_vk_instance_layers, predicate) != k_available_vk_instance_layers.end();
}

[[nodiscard]] std::span<const std::unique_ptr<VulkanPhysicalDevice>> VulkanInstance::EnumeratePhysicalDevices() const {
  return m_vk_physical_devices;
}

void VulkanInstance::PopulatePhysicalDeviceList() {
  u32 device_count;
  std::vector<VkPhysicalDevice> vk_physical_devices{};
  vkEnumeratePhysicalDevices(m_vk_instance, &device_count, nullptr);
  vk_physical_devices.resize(device_count);
  vkEnumeratePhysicalDevices(m_vk_instance, &device_count, vk_physical_devices.data());

  for(VkPhysicalDevice physical_device : vk_physical_devices) {
    m_vk_physical_devices.emplace_back(std::make_unique<VulkanPhysicalDevice>(physical_device));
  }
}

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

}  // namespace mgpu::vulkan
