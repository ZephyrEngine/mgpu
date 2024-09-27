
#pragma once

#include <vulkan/vulkan.h>

#include "backend/texture_view.hpp"
#include "device.hpp"
#include "texture.hpp"

namespace mgpu::vulkan {

class TextureView final : public TextureViewBase {
  public:
   ~TextureView() override;

    static Result<TextureViewBase*> Create(Device* device, Texture* texture, const MGPUTextureViewCreateInfo& create_info);

    [[nodiscard]] VkImageView Handle() { return m_vk_image_view; }
    [[nodiscard]] Texture* GetTexture() { return m_texture; }

  private:
    TextureView(Device* device, Texture* texture, VkImageView vk_image_view, const MGPUTextureViewCreateInfo& create_info);

    Device* m_device{};
    Texture* m_texture{};
    VkImageView m_vk_image_view{};
};

}  // namespace mgpu::vulkan
