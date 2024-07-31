
#pragma once

#include <vulkan/vulkan.h>

#include "backend/texture_view.hpp"
#include "device.hpp"
#include "texture.hpp"

namespace mgpu::vulkan {

class TextureView final : public TextureViewBase {
  public:
   ~TextureView() override;

    static Result<TextureViewBase*> Create(Device* device, VkImage vk_image, const MGPUTextureViewCreateInfo& create_info);

  private:
    TextureView(Device* device, VkImageView vk_image_view, const MGPUTextureViewCreateInfo& create_info);

    Device* m_device{};
    VkImageView m_vk_image_view{};
};

}  // namespace mgpu::vulkan
