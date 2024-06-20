
#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "common/result.hpp"

namespace mgpu {

class VulkanCommandPool;

class VulkanCommandBuffer {
  public:
    static Result<std::unique_ptr<VulkanCommandBuffer>> Create(VkDevice vk_device, std::shared_ptr<VulkanCommandPool> vk_command_pool);

   ~VulkanCommandBuffer();

    [[nodiscard]] VkCommandBuffer Handle() {
      return m_vk_command_buffer;
    }

  private:
    VulkanCommandBuffer(VkDevice vk_device, VkCommandBuffer vk_command_buffer, std::shared_ptr<VulkanCommandPool> vk_command_pool);

    VkDevice m_vk_device;
    VkCommandBuffer m_vk_command_buffer;
    std::shared_ptr<VulkanCommandPool> m_vk_command_pool;
};

}  // namespace mgpu