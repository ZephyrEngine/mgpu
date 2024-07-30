
#pragma once

#include <mgpu/mgpu.h>

#include <atom/panic.hpp>
#include <vulkan/vulkan.h>

namespace mgpu::vulkan {

static inline VkBufferUsageFlags MGPUBufferUsageToVkBufferUsage(MGPUBufferUsage buffer_usage) {
  VkBufferUsageFlags vk_buffer_usage{};
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_SRC)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_DST)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_UNIFORM_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_STORAGE_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDEX_BUFFER)    vk_buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_VERTEX_BUFFER)   vk_buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDIRECT_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  return vk_buffer_usage;
}

static inline VkImageType MGPUTextureTypeToVkImageType(MGPUTextureType texture_type) {
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D: return VK_IMAGE_TYPE_1D;
    case MGPU_TEXTURE_TYPE_2D: return VK_IMAGE_TYPE_2D;
    case MGPU_TEXTURE_TYPE_3D: return VK_IMAGE_TYPE_3D;
    default: ATOM_PANIC("unhandled texture type: {}", (int)texture_type);
  }
}

static inline VkFormat MGPUTextureFormatToVkFormat(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

static inline MGPUTextureAspect MGPUTextureFormatToMGPUTextureAspect(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return MGPU_TEXTURE_ASPECT_COLOR;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

static inline VkImageUsageFlags MGPUTextureUsageToVkImageUsage(MGPUTextureFormat texture_format, MGPUTextureUsage texture_usage) {
  VkImageUsageFlags vk_image_usage{};
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_SRC) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_DST) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_SAMPLED)  vk_image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT) {
    if(MGPUTextureFormatToMGPUTextureAspect(texture_format) & MGPU_TEXTURE_ASPECT_COLOR) {
      vk_image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else {
      // TODO(fleroviux): validate that this always correct.
      vk_image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
  }
  return vk_image_usage;
}

}  // namespace mgpu::vulkan
