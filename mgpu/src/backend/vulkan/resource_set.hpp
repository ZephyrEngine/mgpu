
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>
#include <span>
#include <vector>

#include "backend/resource_set.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device;
class Buffer;
class Texture;

class ResourceSet : public ResourceSetBase {
  public:
   ~ResourceSet() override;

    static Result<ResourceSetBase*> Create(Device* device, const MGPUResourceSetCreateInfo& create_info);

    [[nodiscard]] VkDescriptorSet Handle() { return m_vk_descriptor_set; }

    // TODO(fleroviux): just use a single getter for the BoundResources struct?

    [[nodiscard]] std::span<TextureView* const> GetBoundSampledTextureViews() {
      return m_bound_resources.m_sampled_textures;
    }

    [[nodiscard]] std::span<TextureView* const> GetBoundStorageTextureViews() {
      return m_bound_resources.m_storage_textures;
    }

    [[nodiscard]] std::span<Buffer* const> GetBoundUniformBuffers() {
      return m_bound_resources.m_uniform_buffers;
    }

    [[nodiscard]] std::span<Buffer* const> GetBoundStorageBuffers() {
      return m_bound_resources.m_storage_buffers;
    }

  private:
    struct BoundResources {
      // TODO(fleroviux): use sets instead?
      std::vector<TextureView*> m_sampled_textures{};
      std::vector<TextureView*> m_storage_textures{};
      std::vector<Buffer*> m_uniform_buffers{};
      std::vector<Buffer*> m_storage_buffers{};
    };

    ResourceSet(Device* device, VkDescriptorPool vk_descriptor_pool, VkDescriptorSet vk_descriptor_set, BoundResources bound_resources);

    Device* m_device;
    VkDescriptorPool m_vk_descriptor_pool;
    VkDescriptorSet m_vk_descriptor_set;
    BoundResources m_bound_resources;
};

} // namespace mgpu::vulkan
