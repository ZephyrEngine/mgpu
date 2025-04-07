
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/resource_set.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device;

class ResourceSet : public ResourceSetBase {
  public:
   ~ResourceSet() override;

    static Result<ResourceSetBase*> Create(Device* device, const MGPUResourceSetCreateInfo& create_info);

    [[nodiscard]] VkDescriptorSet Handle() { return m_vk_descriptor_set; }

  private:
    ResourceSet(Device* device, VkDescriptorSet vk_descriptor_set);

    Device* m_device;
    VkDescriptorSet m_vk_descriptor_set;
};

} // namespace mgpu::vulkan
