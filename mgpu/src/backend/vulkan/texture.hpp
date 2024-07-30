
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/texture.hpp"
#include "common/result.hpp"
#include "device.hpp"

namespace mgpu::vulkan {

class Texture final : public TextureBase {
  public:
   ~Texture() override;

    static Result<TextureBase*> Create(Device* device, const MGPUTextureCreateInfo& create_info);

  private:
    Texture(Device* device, VkImage vk_image, VmaAllocation vma_allocation, const MGPUTextureCreateInfo& create_info);

    Device* m_device{};
    VkImage m_vk_image{};
    VmaAllocation m_vma_allocation{};
};

}  // namespace mgpu::vulkan
