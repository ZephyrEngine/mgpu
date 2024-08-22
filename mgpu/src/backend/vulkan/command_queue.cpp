
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "common/result.hpp"

#include "command_queue.hpp"

namespace mgpu::vulkan {

// TODO(fleroviux): keep multiple command buffers around to a) avoid stalling and b) allow submission of multiple, smaller command buffers.

CommandQueue::CommandQueue(
  VkDevice vk_device,
  VkQueue vk_queue,
  VkCommandPool vk_cmd_pool,
  VkCommandBuffer vk_cmd_buffer,
  VkFence vk_cmd_buffer_fence
)   : m_vk_device{vk_device}
    , m_vk_queue{vk_queue}
    , m_vk_cmd_pool{vk_cmd_pool}
    , m_vk_cmd_buffer{vk_cmd_buffer}
    , m_vk_cmd_buffer_fence{vk_cmd_buffer_fence} {
  const VkCommandBufferBeginInfo vk_cmd_buffer_begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  vkBeginCommandBuffer(m_vk_cmd_buffer, &vk_cmd_buffer_begin_info);
}

CommandQueue::~CommandQueue() {
  Flush();

  vkDestroyFence(m_vk_device, m_vk_cmd_buffer_fence, nullptr);
  vkFreeCommandBuffers(m_vk_device, m_vk_cmd_pool, 1u, &m_vk_cmd_buffer);
  vkDestroyCommandPool(m_vk_device, m_vk_cmd_pool, nullptr);
}

Result<std::unique_ptr<CommandQueue>> CommandQueue::Create(VkDevice vk_device, const PhysicalDevice::QueueFamilyIndices& queue_family_indices) {
  const u32 queue_family_index = queue_family_indices.graphics_and_compute.value();

  VkQueue vk_queue{};
  vkGetDeviceQueue(vk_device, queue_family_index, 0u, &vk_queue);

  VkCommandPool vk_cmd_pool{};
  VkCommandBuffer vk_cmd_buffer{};
  VkFence vk_cmd_buffer_fence{};

  const VkCommandPoolCreateInfo vk_cmd_pool_create_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = queue_family_index
  };
  MGPU_VK_FORWARD_ERROR(vkCreateCommandPool(vk_device, &vk_cmd_pool_create_info, nullptr, &vk_cmd_pool));

  const VkCommandBufferAllocateInfo vk_cmd_buffer_alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = vk_cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1u
  };
  MGPU_VK_FORWARD_ERROR(vkAllocateCommandBuffers(vk_device, &vk_cmd_buffer_alloc_info, &vk_cmd_buffer));

  const VkFenceCreateInfo vk_fence_create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0
  };
  MGPU_VK_FORWARD_ERROR(vkCreateFence(vk_device, &vk_fence_create_info, nullptr, &vk_cmd_buffer_fence));

  return std::unique_ptr<CommandQueue>{new CommandQueue{vk_device, vk_queue, vk_cmd_pool, vk_cmd_buffer, vk_cmd_buffer_fence}};
}

MGPUResult CommandQueue::SubmitCommandList(const CommandList* command_list) {
  const CommandBase* command = command_list->GetListHead();

  while(command != nullptr) {
    const CommandType command_type = command->m_command_type;

    switch(command_type) {
      case CommandType::Test: {
        break;
      }
      default: {
        ATOM_PANIC("mgpu: Vulkan: unhandled command type: {}", (int)command_type);
      }
    }

    command = command->m_next;
  }

  return MGPU_SUCCESS;
}

MGPUResult CommandQueue::Flush() {
  const VkSubmitInfo vk_submit_info{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 0u,
    .pWaitSemaphores = nullptr,
    .pWaitDstStageMask = nullptr,
    .commandBufferCount = 1u,
    .pCommandBuffers = &m_vk_cmd_buffer,
    .signalSemaphoreCount = 0u,
    .pSignalSemaphores = nullptr
  };

  MGPU_VK_FORWARD_ERROR(vkEndCommandBuffer(m_vk_cmd_buffer));
  MGPU_VK_FORWARD_ERROR(vkQueueSubmit(m_vk_queue, 1u, &vk_submit_info, m_vk_cmd_buffer_fence));

  const VkCommandBufferBeginInfo vk_cmd_buffer_begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  MGPU_VK_FORWARD_ERROR(vkWaitForFences(m_vk_device, 1u, &m_vk_cmd_buffer_fence, VK_TRUE, ~0ull));
  MGPU_VK_FORWARD_ERROR(vkResetFences(m_vk_device, 1u, &m_vk_cmd_buffer_fence));
  MGPU_VK_FORWARD_ERROR(vkResetCommandBuffer(m_vk_cmd_buffer, 0u));
  MGPU_VK_FORWARD_ERROR(vkBeginCommandBuffer(m_vk_cmd_buffer, &vk_cmd_buffer_begin_info));

  return MGPU_SUCCESS;
}

}  // namespace mgpu::vulkan
