
#pragma once

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <algorithm>
#include <cstring>
#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "common/result.hpp"
#include "vulkan_physical_device.hpp"
#include "vulkan_result.hpp"

namespace mgpu::vulkan {

class VulkanInstance {
  public:
   ~VulkanInstance();

    static Result<std::unique_ptr<VulkanInstance>> Create(
      const VkApplicationInfo& app_info,
      std::vector<const char*> required_instance_extensions,
      std::vector<const char*> required_instance_layers,
      bool accept_vulkan_portability
    );

    static bool QueryInstanceExtensionSupport(const char* extension_name);
    static bool QueryInstanceLayerSupport(const char* layer_name);
    [[nodiscard]] std::span<const std::unique_ptr<VulkanPhysicalDevice>> EnumeratePhysicalDevices() const;

    [[nodiscard]] VkInstance Handle() {
      return m_vk_instance;
    }

  private:
    explicit VulkanInstance(VkInstance vk_instance);

    void PopulatePhysicalDeviceList();

    VkInstance m_vk_instance{};
    std::vector<std::unique_ptr<VulkanPhysicalDevice>> m_vk_physical_devices{};

    static std::vector<VkExtensionProperties> k_available_vk_instance_extensions;
    static std::vector<VkLayerProperties> k_available_vk_instance_layers;
};

}  // namespace mgpu::vulkan

