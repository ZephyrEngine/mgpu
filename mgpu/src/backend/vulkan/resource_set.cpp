
#include <vector>

#include "lib/vulkan_result.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "sampler.hpp"
#include "resource_set_layout.hpp"
#include "resource_set.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

ResourceSet::ResourceSet(Device* device, VkDescriptorPool vk_descriptor_pool, VkDescriptorSet vk_descriptor_set)
    : m_device{device}
    , m_vk_descriptor_pool{vk_descriptor_pool}
    , m_vk_descriptor_set{vk_descriptor_set} {
}

ResourceSet::~ResourceSet() {
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkDescriptorPool vk_descriptor_pool = m_vk_descriptor_pool;
  VkDescriptorSet vk_descriptor_set = m_vk_descriptor_set;
  device->GetDeleterQueue().Schedule([device, vk_descriptor_pool, vk_descriptor_set]() {
    vkFreeDescriptorSets(device->Handle(), vk_descriptor_pool, 1u, &vk_descriptor_set);
  });
}

Result<ResourceSetBase*> ResourceSet::Create(Device* device, const MGPUResourceSetCreateInfo& create_info) {
  const VkDescriptorSetLayout vk_descriptor_set_layout = ((ResourceSetLayout*)create_info.layout)->Handle();

  // Well. It works, doesn't it?
  static VkDescriptorPool vk_descriptor_pool{};
  if(vk_descriptor_pool == VK_NULL_HANDLE) {
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
      .maxSets = 16384u,
      .poolSizeCount = sizeof(vk_descriptor_pool_sizes) / sizeof(VkDescriptorPoolSize),
      .pPoolSizes = vk_descriptor_pool_sizes
    };
    MGPU_VK_FORWARD_ERROR(vkCreateDescriptorPool(device->Handle(), &vk_descriptor_pool_create_info, nullptr, &vk_descriptor_pool));
  }

  const VkDescriptorSetAllocateInfo vk_allocate_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = vk_descriptor_pool,
    .descriptorSetCount = 1u,
    .pSetLayouts = &vk_descriptor_set_layout
  };

  VkDescriptorSet vk_descriptor_set{};
  MGPU_VK_FORWARD_ERROR(vkAllocateDescriptorSets(device->Handle(), &vk_allocate_info, &vk_descriptor_set));

  std::vector<VkWriteDescriptorSet> vk_descriptor_writes{};
  vk_descriptor_writes.resize(create_info.binding_count);

  for(size_t i = 0u; i < create_info.binding_count; i++) {
    const MGPUResourceSetBinding& mgpu_binding = create_info.bindings[i];

    VkDescriptorImageInfo vk_image_info;
    VkDescriptorBufferInfo vk_buffer_info;

    VkWriteDescriptorSet& vk_descriptor_write = vk_descriptor_writes[i];
    vk_descriptor_write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet = vk_descriptor_set,
      .dstBinding = mgpu_binding.binding,
      .dstArrayElement = 0u,
      .descriptorCount = 1u,
      .descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM,
      .pImageInfo = &vk_image_info,
      .pBufferInfo = &vk_buffer_info,
      .pTexelBufferView = nullptr
    };

    // TODO(fleroviux): this probably could be shortened quite a bit.
    // TODO(fleroviux): we could totally just get this information from the layout
    switch(mgpu_binding.type) {
      case MGPU_RESOURCE_BINDING_TYPE_SAMPLER: {
        vk_image_info = {
          .sampler = ((Sampler*)mgpu_binding.texture.sampler)->Handle(),
          .imageView = VK_NULL_HANDLE,
          .imageLayout = VK_IMAGE_LAYOUT_MAX_ENUM
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
      }
      case MGPU_RESOURCE_BINDING_TYPE_TEXTURE_AND_SAMPLER: {
        vk_image_info = {
          .sampler = ((Sampler*)mgpu_binding.texture.sampler)->Handle(),
          .imageView = ((TextureView*)mgpu_binding.texture.texture_view)->Handle(),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
      }
      case MGPU_RESOURCE_BINDING_TYPE_SAMPLED_TEXTURE: {
        vk_image_info = {
          .sampler = VK_NULL_HANDLE,
          .imageView = ((TextureView*)mgpu_binding.texture.texture_view)->Handle(),
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
      }
      case MGPU_RESOURCE_BINDING_TYPE_STORAGE_TEXTURE: {
        vk_image_info = {
          .sampler = VK_NULL_HANDLE,
          .imageView = ((TextureView*)mgpu_binding.texture.texture_view)->Handle(),
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
      }
      case MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER: {
        vk_buffer_info = {
          .buffer = ((Buffer*)mgpu_binding.buffer.buffer)->Handle(),
          .offset = mgpu_binding.buffer.offset,
          .range = mgpu_binding.buffer.size
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
      }
      case MGPU_RESOURCE_BINDING_TYPE_STORAGE_BUFFER: {
        vk_buffer_info = {
          .buffer = ((Buffer*)mgpu_binding.buffer.buffer)->Handle(),
          .offset = mgpu_binding.buffer.offset,
          .range = mgpu_binding.buffer.size
        };
        vk_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
      }
      default: {
        ATOM_PANIC("unknown resource binding type: {}", (int)mgpu_binding.type);
      }
    }
  }

  vkUpdateDescriptorSets(device->Handle(), (u32)vk_descriptor_writes.size(), vk_descriptor_writes.data(), 0u, nullptr);
  return new ResourceSet{device, vk_descriptor_pool, vk_descriptor_set};
}

} // namespace mgpu::vulkan
