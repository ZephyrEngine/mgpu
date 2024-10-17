
#include <atom/panic.hpp>

#include "backend/vulkan/lib/vulkan_result.hpp"

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
  const bool have_depth_stencil_attachment = command->m_have_depth_stencil_attachment;

  // Create a temporary render pass
  atom::Vector_N<VkAttachmentDescription, limits::max_total_attachments> vk_attachment_descriptions{};
  atom::Vector_N<VkAttachmentReference, limits::max_total_attachments> vk_color_attachment_references{};
  VkAttachmentReference vk_depth_stencil_attachment_reference;

  for(const auto& color_attachment : command->m_color_attachments) {
    vk_attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(color_attachment.texture_view->Format()),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = MGPULoadOpToVkAttachmentLoadOp(color_attachment.load_op),
      .storeOp = MGPUStoreOpToVkAttachmentStoreOp(color_attachment.store_op),
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });

    vk_color_attachment_references.PushBack({
      .attachment = (u32)vk_color_attachment_references.Size(),
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });
  }

  if(have_depth_stencil_attachment) {
    const auto& depth_stencil_attachment = command->m_depth_stencil_attachment;

    vk_attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(depth_stencil_attachment.texture_view->Format()),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = MGPULoadOpToVkAttachmentLoadOp(depth_stencil_attachment.depth_load_op),
      .storeOp = MGPUStoreOpToVkAttachmentStoreOp(depth_stencil_attachment.depth_store_op),
      .stencilLoadOp = MGPULoadOpToVkAttachmentLoadOp(depth_stencil_attachment.stencil_load_op),
      .stencilStoreOp = MGPUStoreOpToVkAttachmentStoreOp(depth_stencil_attachment.stencil_store_op),
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    });

    vk_depth_stencil_attachment_reference = {
      .attachment = (u32)vk_color_attachment_references.Size(),
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
  }

  const VkSubpassDescription vk_subpass_description{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = (u32)vk_color_attachment_references.Size(),
    .pColorAttachments = vk_color_attachment_references.Data(),
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = have_depth_stencil_attachment ? &vk_depth_stencil_attachment_reference : nullptr,
    .preserveAttachmentCount = 0u,
    .pPreserveAttachments = nullptr
  };

  const VkRenderPassCreateInfo vk_render_pass_create_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = (u32)vk_attachment_descriptions.Size(),
    .pAttachments = vk_attachment_descriptions.Data(),
    .subpassCount = 1u,
    .pSubpasses = &vk_subpass_description,
    .dependencyCount = 0u,
    .pDependencies = nullptr
  };

  VkRenderPass vk_render_pass{};
  if(vkCreateRenderPass(m_vk_device, &vk_render_pass_create_info, nullptr, &vk_render_pass) != VK_SUCCESS) {
    // TODO(fleroviux): report error to user
    ATOM_PANIC("failed to create VkRenderPass");
  }

  // Create a temporary framebuffer
  atom::Vector_N<VkImageView, limits::max_total_attachments> vk_attachment_image_views{};

  for(const auto& color_attachment : command->m_color_attachments) {
    const auto texture_view = (TextureView*)color_attachment.texture_view;
    vk_attachment_image_views.PushBack(texture_view->Handle());
    state.render_pass.color_attachments.PushBack(texture_view);
  }
  if(have_depth_stencil_attachment) {
    const auto texture_view = (TextureView*)command->m_depth_stencil_attachment.texture_view;
    vk_attachment_image_views.PushBack(texture_view->Handle());
    state.render_pass.depth_stencil_attachment = texture_view;
  }

  const TextureViewBase* canonical_texture_view = have_depth_stencil_attachment ? command->m_depth_stencil_attachment.texture_view : command->m_color_attachments.Front().texture_view;
  const MGPUExtent3D canonical_texture_extent = ((TextureView*)canonical_texture_view)->GetTexture()->Extent();

  const VkFramebufferCreateInfo vk_framebuffer_create_info{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = vk_render_pass,
    .attachmentCount = (u32)vk_attachment_image_views.Size(),
    .pAttachments = vk_attachment_image_views.Data(),
    .width = canonical_texture_extent.width,
    .height = canonical_texture_extent.height,
    .layers = 1u
  };

  VkFramebuffer vk_framebuffer{};
  if(vkCreateFramebuffer(m_vk_device, &vk_framebuffer_create_info, nullptr, &vk_framebuffer) != VK_SUCCESS) {
    // TODO(fleroviux): report error to user
    ATOM_PANIC("failed to create VkFramebuffer");
  }

  // Begin the render pass
  atom::Vector_N<VkClearValue, limits::max_total_attachments> vk_clear_values{};
  for(const auto& color_attachment : command->m_color_attachments) {
    // TODO(fleroviux): implement code paths for unsigned and signed integer texture formats
    vk_clear_values.PushBack({
      .color = {
        .float32 = {
          (float)color_attachment.clear_color.r,
          (float)color_attachment.clear_color.g,
          (float)color_attachment.clear_color.b,
          (float)color_attachment.clear_color.a
        }
      }
    });
  }

  if(have_depth_stencil_attachment) {
    vk_clear_values.PushBack({
      .depthStencil = {
        .depth = command->m_depth_stencil_attachment.clear_depth,
        .stencil = command->m_depth_stencil_attachment.clear_stencil
      }
    });
  }

  const VkRenderPassBeginInfo vk_render_pass_begin_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = nullptr,
    .renderPass = vk_render_pass,
    .framebuffer = vk_framebuffer,
    .renderArea = {
      .offset = { .x = 0, .y = 0 },
      .extent = { .width = canonical_texture_extent.width, .height = canonical_texture_extent.height }
    },
    .clearValueCount = (u32)vk_clear_values.Size(),
    .pClearValues = vk_clear_values.Data()
  };
  vkCmdBeginRenderPass(m_vk_cmd_buffer, &vk_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

  // Destroy temporary render pass and framebuffer at the end of the frame.
  // TODO
}

void CommandQueue::HandleCmdEndRenderPass(CommandListState& state) {
  vkCmdEndRenderPass(m_vk_cmd_buffer);

  state.render_pass = {};
}

}  // namespace mgpu::vulkan
