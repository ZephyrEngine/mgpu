
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

TextureView::TextureView(Device* device, Texture* texture, VkImageView vk_image_view, const MGPUTextureViewCreateInfo& create_info)
    : TextureViewBase{create_info}
    , m_device{device}
    , m_texture{texture}
    , m_vk_image_view{vk_image_view} {
}

TextureView::~TextureView() {
  // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkImageView vk_image_view = m_vk_image_view;
  device->GetDeleterQueue().Schedule([device, vk_image_view]() {
    vkDestroyImageView(device->Handle(), vk_image_view, nullptr);
  });
}

Result<TextureViewBase*> TextureView::Create(Device* device, Texture* texture, const MGPUTextureViewCreateInfo& create_info) {
  const VkImageViewCreateInfo vk_image_view_create_info{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .image = texture->Handle(),
    .viewType = MGPUTextureViewTypeToVkImageViewType(create_info.type),
    .format = MGPUTextureFormatToVkFormat(create_info.format),
    .components = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange = {
      .aspectMask = MGPUTextureAspectToVkImageAspect(create_info.aspect),
      .baseMipLevel = create_info.base_mip,
      .levelCount = create_info.mip_count,
      .baseArrayLayer = create_info.base_array_layer,
      .layerCount = create_info.array_layer_count
    }
  };

  VkImageView vk_image_view{};
  MGPU_VK_FORWARD_ERROR(vkCreateImageView(device->Handle(), &vk_image_view_create_info, nullptr, &vk_image_view));
  return new TextureView{device, texture, vk_image_view, create_info};
}

}  // namespace mgpu::vulkan
