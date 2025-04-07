
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/resource_set_layout.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device;

class ResourceSetLayout : public ResourceSetLayoutBase {
  public:
    ~ResourceSetLayout() override;

    static Result<ResourceSetLayoutBase*> Create(Device* device, const MGPUResourceSetLayoutCreateInfo& create_info);

    [[nodiscard]] VkDescriptorSetLayout Handle() { return m_vk_descriptor_set_layout; }

  private:
    ResourceSetLayout(Device* device, VkDescriptorSetLayout vk_descriptor_set_layout);

    Device* m_device;
    VkDescriptorSetLayout m_vk_descriptor_set_layout;
};

} // namespace mgpu::vulkan
