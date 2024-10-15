
#include <vector>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/render_target.hpp"
#include "backend/vulkan/texture.hpp"
#include "common/result.hpp"

#include "command_queue.hpp"
#include "conversion.hpp"

#include "texture_view.hpp"

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

  CommandListState state{};

  while(command != nullptr) {
    const CommandType command_type = command->m_command_type;

    switch(command_type) {
      case CommandType::BeginRenderPass: HandleCmdBeginRenderPass(state, (BeginRenderPassCommand*)command); break;
      case CommandType::EndRenderPass: HandleCmdEndRenderPass(state); break;
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

void CommandQueue::HandleCmdBeginRenderPass(CommandListState& state, const BeginRenderPassCommand* command) {
  const auto render_target = (RenderTarget*)command->m_render_target;
  const MGPUExtent2D render_area = render_target->Extent();

  // TODO: remove hacky hardcoded barriers
  std::vector<VkImageMemoryBarrier> image_memory_barriers{};
  for(auto attachment : render_target->GetAttachments()) {
    image_memory_barriers.push_back({
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .srcQueueFamilyIndex = 0,
      .dstQueueFamilyIndex = 0,
      .image = attachment->GetTexture()->Handle(),
      .subresourceRange = {
        .aspectMask = MGPUTextureAspectToVkImageAspect(attachment->Aspect()),
        .baseMipLevel = 0u,
        .levelCount = 1u,
        .baseArrayLayer = 0u,
        .layerCount = 1u
      }
    });
  }
  vkCmdPipelineBarrier(m_vk_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0u, nullptr, 0u, nullptr, image_memory_barriers.size(), image_memory_barriers.data());

  const VkClearValue clear_value{
    .color = {.float32 = {0.0f, 1.0f, 0.0f, 1.0f}}
  };

  RenderPassQuery render_pass_query{};
  render_pass_query.SetColorAttachmentConfig(0, MGPU_LOAD_OP_CLEAR, MGPU_STORE_OP_STORE);

  // TODO(fleroviux): handle failure to resolve the query to a render pass.
  Result<VkRenderPass> vk_render_pass_result = render_target->GetRenderPass(render_pass_query);

  const VkRenderPassBeginInfo vk_render_pass_begin_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = nullptr,
    .renderPass = vk_render_pass_result.Unwrap(),
    .framebuffer = render_target->Handle(),
    .renderArea = {
      .offset = { .x = 0, .y = 0 },
      .extent = { .width = render_area.width, .height = render_area.height }
    },
    .clearValueCount = 1u, // !!! TODO
    .pClearValues = &clear_value, // !!! TODO
  };

  vkCmdBeginRenderPass(m_vk_cmd_buffer, &vk_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

  state.current_render_target = render_target;
}

void CommandQueue::HandleCmdEndRenderPass(CommandListState& state) {
  const auto render_target = (RenderTarget*)state.current_render_target;

  vkCmdEndRenderPass(m_vk_cmd_buffer);

  // Transition the swap chain image to the PRESENT_SRC_KHR layout required by vkQueueSubmit().
  // We set the dstAccessMask to 0 and dstStageMask to TOP_OF_PIPE_BIT because vkQueueSubmit()
  // will make sure to make any writes to the underlying memory visible to the presentation engine.
  // TODO: remove hacky hardcoded barriers
  std::vector<VkImageMemoryBarrier> image_memory_barriers{};
  for(auto attachment : render_target->GetAttachments()) {
    image_memory_barriers.push_back({
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = 0,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .srcQueueFamilyIndex = 0,
      .dstQueueFamilyIndex = 0,
      .image = attachment->GetTexture()->Handle(),
      .subresourceRange = {
        .aspectMask = MGPUTextureAspectToVkImageAspect(attachment->Aspect()),
        .baseMipLevel = 0u,
        .levelCount = 1u,
        .baseArrayLayer = 0u,
        .layerCount = 1u
      }
    });
  }
  vkCmdPipelineBarrier(m_vk_cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0u, nullptr, 0u, nullptr, image_memory_barriers.size(), image_memory_barriers.data());

  state.current_render_target = nullptr;
}

}  // namespace mgpu::vulkan
