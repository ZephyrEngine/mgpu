
#include "vulkan_command_pool.hpp"

namespace mgpu {

Result<std::unique_ptr<VulkanCommandPool>> VulkanCommandPool::Create(VkDevice vk_device, VkCommandPoolCreateFlags flags, u32 queue_family_index) {
  const VkCommandPoolCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = flags,
    .queueFamilyIndex = queue_family_index
  };

  VkCommandPool vk_command_pool;

  if(vkCreateCommandPool(vk_device, &create_info, nullptr, &vk_command_pool) != VK_SUCCESS) {
    return MGPU_INTERNAL_ERROR;
  }
  return std::unique_ptr<VulkanCommandPool>(new VulkanCommandPool{vk_device, vk_command_pool});
}

VulkanCommandPool::VulkanCommandPool(VkDevice vk_device, VkCommandPool vk_command_pool)
    : m_vk_device{vk_device}
    , m_vk_command_pool{vk_command_pool} {
}

VulkanCommandPool::~VulkanCommandPool() {
  vkDestroyCommandPool(m_vk_device, m_vk_command_pool, nullptr);
}

}  // namespace mgpu
