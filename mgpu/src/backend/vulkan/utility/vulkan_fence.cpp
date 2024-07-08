
#include "vulkan_fence.hpp"
#include "vulkan_result.hpp"

namespace mgpu {

Result<std::unique_ptr<VulkanFence>> VulkanFence::Create(VkDevice vk_device, CreateSignalled create_signalled) {
  const VkFenceCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = create_signalled == CreateSignalled::Yes ? VK_FENCE_CREATE_SIGNALED_BIT : (VkFenceCreateFlagBits)0
  };

  VkFence vk_fence;

  if(VkResult vk_result = vkCreateFence(vk_device, &create_info, nullptr, &vk_fence); vk_result != VK_SUCCESS) {
    return vk_result_to_mgpu_result(vk_result);
  }
  return std::unique_ptr<VulkanFence>{new VulkanFence{vk_device, vk_fence}};
}

VulkanFence::VulkanFence(VkDevice vk_device, VkFence vk_fence)
    : m_vk_device{vk_device}
    , m_vk_fence{vk_fence} {
}

VulkanFence::~VulkanFence() {
  vkDestroyFence(m_vk_device, m_vk_fence, nullptr);
}

}  // namespace mgpu
