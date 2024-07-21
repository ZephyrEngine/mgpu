
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "backend/vulkan/lib/vulkan_instance.hpp"
#include "backend/vulkan/physical_device.hpp"
#include "backend/instance.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Instance final : public InstanceBase {
  public:
   ~Instance() override;

    static Result<InstanceBase*> Create();

    Result<std::span<PhysicalDeviceBase* const>> EnumeratePhysicalDevices() override;

  private:
    explicit Instance(std::unique_ptr<VulkanInstance> vk_instance);

    void BuildPhysicalDeviceList();

    // TODO(fleroviux): improve lifetime management of m_vk_instance (there is an implicit dependency from m_physical_device on the instance)
    std::unique_ptr<VulkanInstance> m_vk_instance{};
    std::vector<PhysicalDevice*> m_physical_devices{};
};

}  // namespace mgpu::vulkan
