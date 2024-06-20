
#include "vulkan_command_buffer.hpp"
#include "vulkan_command_pool.hpp"

namespace mgpu {

Result<std::unique_ptr<VulkanCommandBuffer>> VulkanCommandBuffer::Create(VkDevice vk_device, std::shared_ptr<VulkanCommandPool> vk_command_pool) {
  const VkCommandBufferAllocateInfo alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = vk_command_pool->Handle(),
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1u
  };

  VkCommandBuffer vk_command_buffer;

  if(vkAllocateCommandBuffers(vk_device, &alloc_info, &vk_command_buffer) != VK_SUCCESS) {
    return MGPU_INTERNAL_ERROR;
  }
  return std::unique_ptr<VulkanCommandBuffer>{new VulkanCommandBuffer{vk_device, vk_command_buffer, std::move(vk_command_pool)}};
}

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice vk_device, VkCommandBuffer vk_command_buffer, std::shared_ptr<VulkanCommandPool> vk_command_pool)
    : m_vk_device{vk_device}
    , m_vk_command_buffer{vk_command_buffer}
    , m_vk_command_pool{std::move(vk_command_pool)} {
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
  vkFreeCommandBuffers(m_vk_device, m_vk_command_pool->Handle(), 1u, &m_vk_command_buffer);
}

}  // namespace mgpu
