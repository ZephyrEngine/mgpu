
#include "lib/vulkan_result.hpp"
#include "device.hpp"
#include "resource_set_layout.hpp"
#include "resource_set.hpp"

namespace mgpu::vulkan {

ResourceSet::ResourceSet(Device* device, VkDescriptorSet vk_descriptor_set)
    : m_device{device}
    , m_vk_descriptor_set{vk_descriptor_set} {
}

ResourceSet::~ResourceSet() {
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkDescriptorSet vk_descriptor_set = m_vk_descriptor_set;
  device->GetDeleterQueue().Schedule([device, vk_descriptor_set]() {
    // TODO ...
  });
}

Result<ResourceSetBase*> ResourceSet::Create(Device* device, const MGPUResourceSetCreateInfo& create_info) {
  const VkDescriptorSetLayout vk_descriptor_set_layout = ((ResourceSetLayout*)create_info.layout)->Handle();

  // Well. It works, doesn't it?
  const VkDescriptorPoolSize vk_descriptor_pool_sizes[] {
    {
      .type = VK_DESCRIPTOR_TYPE_SAMPLER,
      .descriptorCount = 65536u
    },
    {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 65536u
    },
    {
      .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .descriptorCount = 65536u
    },
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .descriptorCount = 65536u
    },
    {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 65536u
    },
    {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 65536u
    }
  };
  const VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets = 1u,
    .poolSizeCount = sizeof(vk_descriptor_pool_sizes) / sizeof(VkDescriptorPoolSize),
    .pPoolSizes = vk_descriptor_pool_sizes
  };
  VkDescriptorPool vk_descriptor_pool{};
  MGPU_VK_FORWARD_ERROR(vkCreateDescriptorPool(device->Handle(), &vk_descriptor_pool_create_info, nullptr, &vk_descriptor_pool));

  const VkDescriptorSetAllocateInfo vk_allocate_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = vk_descriptor_pool,
    .descriptorSetCount = 1u,
    .pSetLayouts = &vk_descriptor_set_layout
  };

  VkDescriptorSet vk_descriptor_set{};
  MGPU_VK_FORWARD_ERROR(vkAllocateDescriptorSets(device->Handle(), &vk_allocate_info, &vk_descriptor_set));
  // TODO(fleroviux): bind resources
  return new ResourceSet{device, vk_descriptor_set};
}

} // namespace mgpu::vulkan
