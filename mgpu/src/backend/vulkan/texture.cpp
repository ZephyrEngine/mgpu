
#include <algorithm>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "common/texture.hpp"
#include "conversion.hpp"
#include "texture.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

static const VmaAllocationCreateInfo vma_alloc_info = {
  .usage = VMA_MEMORY_USAGE_AUTO,
  .flags = 0
};

bool Texture::State::operator==(const State& other_state) const {
  return m_image_layout == other_state.m_image_layout &&
         m_access == other_state.m_access &&
         m_pipeline_stages == other_state.m_pipeline_stages;
}

Texture::Texture(Device* device, VkImage vk_image, VmaAllocation vma_allocation, const MGPUTextureCreateInfo& create_info)
    : TextureBase{create_info}
    , m_device{device}
    , m_vk_image{vk_image}
    , m_vma_allocation{vma_allocation} {
}

Texture::~Texture() {
  if(m_vma_allocation != nullptr) { // When m_vma_allocation is null the VkImage is not owned by this texture.
    // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
    // TODO(fleroviux): make this a little bit less verbose.
    Device *device = m_device;
    VkImage vk_image = m_vk_image;
    VmaAllocation vma_allocation = m_vma_allocation;
    device->GetDeleterQueue().Schedule([device, vk_image, vma_allocation]() {
      vkDestroyImage(device->Handle(), vk_image, nullptr);
      vmaFreeMemory(device->GetVmaAllocator(), vma_allocation);
    });
  }
}

Result<TextureBase*> Texture::Create(Device* device, const MGPUTextureCreateInfo& create_info) {
  const MGPUTextureType type = create_info.type;
  const u32 width = create_info.extent.width;
  const u32 height = create_info.extent.height;
  const u32 depth = create_info.extent.depth;
  const u32 mip_count = std::max<u32>(create_info.mip_count, 1u);
  const u32 array_layer_count = std::max<u32>(create_info.array_layer_count, 1u);

  VkImageCreateFlags vk_image_create_flags = 0;

  if(type == MGPU_TEXTURE_TYPE_2D && array_layer_count >= 6u && width == height) {
    // TODO(fleroviux): evaluate the performance implications of enabling this on textures which do not need it.
    vk_image_create_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }

  if(type == MGPU_TEXTURE_TYPE_3D) {
    // TODO(fleroviux): evaluate the performance implications of enabling this on textures which do not need it.
    vk_image_create_flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
  }

  const VkImageCreateInfo vk_image_create_info{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = vk_image_create_flags,
    .imageType = MGPUTextureTypeToVkImageType(create_info.type),
    .format = MGPUTextureFormatToVkFormat(create_info.format),
    .extent = {
      .width = width,
      .height = height,
      .depth = depth
    },
    .mipLevels = mip_count,
    .arrayLayers = array_layer_count,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = MGPUTextureUsageToVkImageUsage(create_info.format, create_info.usage),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0u,
    .pQueueFamilyIndices = nullptr,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };

  VkImage vk_image;
  VmaAllocation vma_allocation;
  MGPU_VK_FORWARD_ERROR(vmaCreateImage(device->GetVmaAllocator(), &vk_image_create_info, &vma_alloc_info, &vk_image, &vma_allocation, nullptr));
  return new Texture{device, vk_image, vma_allocation, create_info};
}

Texture* Texture::FromVkImage(Device* device, const MGPUTextureCreateInfo& create_info, VkImage vk_image) {
  return new Texture{device, vk_image, nullptr, create_info};
}

Result<TextureViewBase*> Texture::CreateView(const MGPUTextureViewCreateInfo& create_info) {
  return TextureView::Create(m_device, this, create_info);
}

void Texture::TransitionState(const State& new_state, VkCommandBuffer vk_command_buffer) {
  if(new_state == m_state) {
    return;
  }

  const VkImageMemoryBarrier vk_image_memory_barrier{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .pNext = nullptr,
    .srcAccessMask = m_state.m_access,
    .dstAccessMask = new_state.m_access,
    .oldLayout = m_state.m_image_layout,
    .newLayout = new_state.m_image_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = m_vk_image,
    .subresourceRange = {
      .aspectMask = MGPUTextureAspectToVkImageAspect(MGPUTextureFormatToMGPUTextureAspect(Format())),
      .baseMipLevel = 0u,
      .levelCount = MipCount(),
      .baseArrayLayer = 0u,
      .layerCount = ArrayLayerCount()
    }
  };

  vkCmdPipelineBarrier(vk_command_buffer, m_state.m_pipeline_stages, new_state.m_pipeline_stages, 0, 0u, nullptr, 0u, nullptr, 1u, &vk_image_memory_barrier);

  m_state = new_state;
}

}  // namespace mgpu::vulkan
