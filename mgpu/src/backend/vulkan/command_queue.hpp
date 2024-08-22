
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <memory>
#include <vulkan/vulkan.h>

#include "backend/vulkan/physical_device.hpp"
#include "backend/command_list.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class CommandQueue : atom::NonCopyable, atom::NonMoveable {
  public:
   ~CommandQueue();

    static Result<std::unique_ptr<CommandQueue>> Create(VkDevice vk_device, const PhysicalDevice::QueueFamilyIndices& queue_family_indices);

    MGPUResult SubmitCommandList(const CommandList* command_list);
    MGPUResult Flush();

  private:
    CommandQueue(
      VkDevice vk_device,
      VkQueue vk_queue,
      VkCommandPool vk_cmd_pool,
      VkCommandBuffer vk_cmd_buffer,
      VkFence vk_cmd_buffer_fence
    );

    VkDevice m_vk_device;
    VkQueue m_vk_queue;
    VkCommandPool m_vk_cmd_pool;
    VkCommandBuffer m_vk_cmd_buffer;
    VkFence m_vk_cmd_buffer_fence;
};

}  // namespace mgpu::vulkan
