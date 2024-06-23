
#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "common/result.hpp"

namespace mgpu {

class VulkanFence {
  public:
    enum class CreateSignalled {
      Yes,
      No
    };

    static Result<std::unique_ptr<VulkanFence>> Create(VkDevice vk_device, CreateSignalled create_signalled);

   ~VulkanFence();

    [[nodiscard]] VkFence Handle() {
      return m_vk_fence;
    }

  private:
    VulkanFence(VkDevice vk_device, VkFence vk_fence);

    VkDevice m_vk_device;
    VkFence m_vk_fence;
};

}  // namespace mgpu
