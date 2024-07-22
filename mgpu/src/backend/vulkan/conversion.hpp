
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

namespace mgpu::vulkan {

static inline VkBufferUsageFlags MGPUBufferUsageToVkBufferUsage(MGPUBufferUsage buffer_usage) {
  VkBufferUsageFlags vk_buffer_usage{};
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_SRC) vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_DST) vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_UNIFORM_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_STORAGE_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDEX_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_VERTEX_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDIRECT_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  return vk_buffer_usage;
}

}  // namespace mgpu::vulkan
