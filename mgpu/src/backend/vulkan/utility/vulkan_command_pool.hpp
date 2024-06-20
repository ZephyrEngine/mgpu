
#pragma once

#include <atom/integer.hpp>
#include <vulkan/vulkan.h>
#include <memory>

#include "common/result.hpp"

namespace mgpu {

class VulkanCommandPool {
  public:
    static Result<std::unique_ptr<VulkanCommandPool>> Create(VkDevice vk_device, VkCommandPoolCreateFlags flags, u32 queue_family_index);

   ~VulkanCommandPool();

    [[nodiscard]] VkCommandPool Handle() {
      return m_vk_command_pool;
    }

  private:
    VulkanCommandPool(VkDevice vk_device, VkCommandPool vk_command_pool);

    VkDevice m_vk_device;
    VkCommandPool m_vk_command_pool;
};

}  // namespace mgpu
