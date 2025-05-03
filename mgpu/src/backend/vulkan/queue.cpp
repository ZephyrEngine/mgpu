
#include <atom/float.hpp>
#include <atom/panic.hpp>
#include <cstring>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "common/texture.hpp"

#include "pipeline_state/color_blend_state.hpp"
#include "pipeline_state/depth_stencil_state.hpp"
#include "pipeline_state/input_assembly_state.hpp"
#include "pipeline_state/rasterizer_state.hpp"
#include "pipeline_state/shader_module.hpp"
#include "pipeline_state/shader_program.hpp"
#include "pipeline_state/vertex_input_state.hpp"
#include "buffer.hpp"
#include "conversion.hpp"
#include "queue.hpp"
#include "resource_set.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

Queue::Queue(
  VkDevice vk_device,
  VkQueue vk_queue,
  VkCommandPool vk_cmd_pool,
  std::vector<FencedCommandBuffer> fenced_cmd_buffers,
  std::shared_ptr<DeleterQueue> deleter_queue,
  std::shared_ptr<RenderPassCache> render_pass_cache
)   : m_vk_device{vk_device}
    , m_vk_queue{vk_queue}
    , m_vk_cmd_pool{vk_cmd_pool}
    , m_fenced_cmd_buffers{std::move(fenced_cmd_buffers)}
    , m_deleter_queue{deleter_queue}
    , m_render_pass_cache{std::move(render_pass_cache)}
    , m_graphics_pipeline_cache{vk_device, std::move(deleter_queue)} {
  BeginNextCommandBuffer();
}

Queue::~Queue() {
  Flush();

  for(const auto& fenced_cmd_buffer : m_fenced_cmd_buffers) {
    if(fenced_cmd_buffer.submitted) {
      vkWaitForFences(m_vk_device, 1u, &fenced_cmd_buffer.vk_fence, VK_TRUE, ~0ull);
    }
    vkDestroyFence(m_vk_device, fenced_cmd_buffer.vk_fence, nullptr);
    vkFreeCommandBuffers(m_vk_device, m_vk_cmd_pool, 1u, &fenced_cmd_buffer.vk_cmd_buffer);
  }
  vkDestroyCommandPool(m_vk_device, m_vk_cmd_pool, nullptr);
}

Result<std::unique_ptr<Queue>> Queue::Create(
  VkDevice vk_device,
  u32 queue_family_index,
  std::shared_ptr<DeleterQueue> deleter_queue,
  std::shared_ptr<RenderPassCache> render_pass_cache
) {
  VkQueue vk_queue{};
  vkGetDeviceQueue(vk_device, queue_family_index, 0u, &vk_queue);

  VkCommandPool vk_cmd_pool{};

  const VkCommandPoolCreateInfo vk_cmd_pool_create_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = queue_family_index
  };
  MGPU_VK_FORWARD_ERROR(vkCreateCommandPool(vk_device, &vk_cmd_pool_create_info, nullptr, &vk_cmd_pool));

  // TODO: how many command buffers is ideal? Make it dependent on the number of swap chain images?
  static constexpr size_t k_command_buffer_count = 16u;

  const VkCommandBufferAllocateInfo vk_cmd_buffer_alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = vk_cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1u
  };

  const VkFenceCreateInfo vk_fence_create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0
  };

  std::vector<FencedCommandBuffer> fenced_cmd_buffers{};

  for(size_t i = 0; i < k_command_buffer_count; i++) {
    // TODO(fleroviux): if this fails we will leak memory
    VkCommandBuffer vk_cmd_buffer{};
    MGPU_VK_FORWARD_ERROR(vkAllocateCommandBuffers(vk_device, &vk_cmd_buffer_alloc_info, &vk_cmd_buffer));

    // TODO(fleroviux): if this fails we will leak memory
    VkFence vk_fence{};
    MGPU_VK_FORWARD_ERROR(vkCreateFence(vk_device, &vk_fence_create_info, nullptr, &vk_fence));

    fenced_cmd_buffers.push_back({vk_cmd_buffer, vk_fence});
  }

  return std::unique_ptr<Queue>{new Queue{
    vk_device,
    vk_queue,
    vk_cmd_pool,
    std::move(fenced_cmd_buffers),
    std::move(deleter_queue),
    std::move(render_pass_cache)
  }};
}

void Queue::SetDevice(Device* device) {
  m_device = device;
}

void Queue::SetSwapChainAcquireSemaphore(VkSemaphore vk_swap_chain_acquire_semaphore) {
  // If we still have another semaphore around for some reason, destroy it now.
  DestroySwapChainAcquireSemaphore();
  
  m_vk_swap_chain_acquire_semaphore = vk_swap_chain_acquire_semaphore;
}

MGPUResult Queue::Present(SwapChain* swap_chain, u32 texture_index) {
  VkSwapchainKHR vk_swap_chain = swap_chain->Handle();
  VkSemaphore vk_swap_chain_acquire_semaphore = m_vk_swap_chain_acquire_semaphore;

  VkPresentInfoKHR vk_present_info{
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = nullptr,
    .waitSemaphoreCount = 1u,
    .pWaitSemaphores = &vk_swap_chain_acquire_semaphore,
    .swapchainCount = 1u,
    .pSwapchains = &vk_swap_chain,
    .pImageIndices = &texture_index,
    .pResults = nullptr
  };

  // TODO(fleroviux): clean this up and ensure that the barrier is correct.
  ((Texture*)swap_chain->EnumerateTextures().Unwrap()[texture_index])->TransitionState({
    .m_image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    .m_access = VK_ACCESS_TRANSFER_READ_BIT,
    .m_pipeline_stages = VK_PIPELINE_STAGE_TRANSFER_BIT
  }, m_vk_cmd_buffer);
  Flush();

  MGPU_VK_FORWARD_ERROR(vkQueuePresentKHR(m_vk_queue, &vk_present_info));

  // Delete the temporary semaphore at the next opportunity.
  DestroySwapChainAcquireSemaphore();

  return MGPU_SUCCESS;
}

MGPUResult Queue::SubmitCommandList(const CommandList* command_list) {
  const CommandBase* command = command_list->GetListHead();

  CommandListState state{};

  while(command != nullptr) {
    const CommandType command_type = command->m_command_type;

    switch(command_type) {
      case CommandType::BeginRenderPass: HandleCmdBeginRenderPass(state, *(BeginRenderPassCommand*)command); break;
      case CommandType::EndRenderPass: HandleCmdEndRenderPass(state); break;
      case CommandType::UseShaderProgram: HandleCmdUseShaderProgram(state, *(UseShaderProgramCommand*)command); break;
      case CommandType::UseRasterizerState: HandleCmdUseRasterizerState(state, *(UseRasterizerStateCommand*)command); break;
      case CommandType::UseInputAssemblyState: HandleCmdUseInputAssemblyState(state, *(UseInputAssemblyStateCommand*)command); break;
      case CommandType::UseColorBlendState: HandleCmdUseColorBlendState(state, *(UseColorBlendStateCommand*)command); break;
      case CommandType::UseVertexInputState: HandleCmdUseVertexInputState(state, *(UseVertexInputStateCommand*)command); break;
      case CommandType::UseDepthStencilState: HandleCmdUseDepthStencilState(state, *(UseDepthStencilStateCommand*)command); break;
      case CommandType::SetViewport: HandleCmdSetViewport(state, *(SetViewportCommand*)command); break;
      case CommandType::SetScissor: HandleCmdSetScissor(state, *(SetScissorCommand*)command); break;
      case CommandType::BindVertexBuffer: HandleCmdBindVertexBuffer(state, *(BindVertexBufferCommand*)command); break;
      case CommandType::BindIndexBuffer: HandleCmdBindIndexBuffer(state, *(BindIndexBufferCommand*)command); break;
      case CommandType::BindResourceSet: HandleCmdBindResourceSet(state, *(BindResourceSetCommand*)command); break;
      case CommandType::Draw: HandleCmdDraw(state, *(DrawCommand*)command); break;
      case CommandType::DrawIndexed: HandleCmdDrawIndexed(state, *(DrawIndexedCommand*)command); break;
      default: {
        ATOM_PANIC("mgpu: Vulkan: unhandled command type: {}", (int)command_type);
      }
    }

    command = command->m_next;
  }

  return MGPU_SUCCESS;
}

MGPUResult Queue::BufferUpload(const BufferBase* buffer, std::span<const u8> data, u64 offset) {
  const auto dst_buffer = (Buffer*)buffer;

  // Bring the buffer into a state where it's safe to copy to
  dst_buffer->TransitionState({VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT}, m_vk_cmd_buffer);

  // TODO(fleroviux): even though the spec guarantees up to 64kb of data work, it might make sense to set the threshold lower?
  // "The additional cost of this functionality compared to buffer to buffer copies means it is only recommended for very small amounts of data, and is why it is limited to only 65536 bytes."
  if(data.size_bytes() < 65536u && offset % 4u == 0u && data.size_bytes() % 4u == 0u) {
    vkCmdUpdateBuffer(m_vk_cmd_buffer, dst_buffer->Handle(), offset, data.size_bytes(), data.data());
  } else {
    // TODO(fleroviux): instead of allocating a bunch of small, individual buffers, allocate a single, large arena staging buffer
    Result<BufferBase*> staging_buffer_result = Buffer::Create(m_device, {
      .size = (u64)data.size_bytes(),
      .usage = MGPU_BUFFER_USAGE_COPY_SRC,
      .flags = MGPU_BUFFER_FLAGS_HOST_VISIBLE
    });
    MGPU_FORWARD_ERROR(staging_buffer_result.Code());

    const auto staging_buffer = (Buffer*)staging_buffer_result.Unwrap();

    Result<void*> map_address_result = staging_buffer->Map();
    MGPU_FORWARD_ERROR(map_address_result.Code());

    void* map_address = map_address_result.Unwrap();
    std::memcpy(map_address, data.data(), data.size_bytes());
    staging_buffer->FlushRange(0u, MGPU_WHOLE_SIZE);

    // Perform a copy from our staging buffer to the destination buffer
    const VkBufferCopy vk_buffer_copy{
      .srcOffset = 0u,
      .dstOffset = offset,
      .size = data.size_bytes()
    };
    vkCmdCopyBuffer(m_vk_cmd_buffer, staging_buffer->Handle(), dst_buffer->Handle(), 1u, &vk_buffer_copy);

    delete staging_buffer;
  }

  return MGPU_SUCCESS;
}

MGPUResult Queue::TextureUpload(const TextureBase* texture, const MGPUTextureUploadRegion& region, const void* data) {
  const auto dst_texture = (Texture*)texture;

  const size_t size_bytes = MGPUTextureFormatGetTexelSize(texture->Format()) * region.extent.width * region.extent.height * region.extent.depth;

  // TODO(fleroviux): instead of allocating a bunch of small, individual buffers, allocate a single, large arena staging buffer
  Result<BufferBase*> staging_buffer_result = Buffer::Create(m_device, {
    .size = (u64)size_bytes,
    .usage = MGPU_BUFFER_USAGE_COPY_SRC,
    .flags = MGPU_BUFFER_FLAGS_HOST_VISIBLE
  });
  MGPU_FORWARD_ERROR(staging_buffer_result.Code());

  const auto staging_buffer = (Buffer*)staging_buffer_result.Unwrap();

  Result<void*> map_address_result = staging_buffer->Map();
  MGPU_FORWARD_ERROR(map_address_result.Code());

  void* map_address = map_address_result.Unwrap();
  std::memcpy(map_address, data, size_bytes);
  staging_buffer->FlushRange(0u, MGPU_WHOLE_SIZE);

  dst_texture->TransitionState({
    .m_image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .m_access = VK_ACCESS_TRANSFER_WRITE_BIT,
    .m_pipeline_stages = VK_PIPELINE_STAGE_TRANSFER_BIT
  }, m_vk_cmd_buffer);

  const VkBufferImageCopy vk_buffer_image_copy{
    .bufferOffset = 0u,
    .bufferRowLength = 0u,
    .bufferImageHeight = 0u,
    .imageSubresource = {
      .aspectMask = MGPUTextureFormatToMGPUTextureAspect(texture->Format()),
      .mipLevel = region.mip_level,
      .baseArrayLayer = region.base_array_layer,
      .layerCount = region.array_layer_count
    },
    .imageOffset = MGPUOffset3DToVkOffset3D(region.offset),
    .imageExtent = MGPUExtent3DToVkExtent3D(region.extent)
  };
  vkCmdCopyBufferToImage(m_vk_cmd_buffer, staging_buffer->Handle(), dst_texture->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &vk_buffer_image_copy);

  delete staging_buffer;
  return MGPU_SUCCESS;
}

MGPUResult Queue::Flush() {
  // TODO: begin and submit command buffers on demand instead?
  MGPU_FORWARD_ERROR(SubmitCurrentCommandBuffer());
  MGPU_FORWARD_ERROR(BeginNextCommandBuffer());
  return MGPU_SUCCESS;
}

MGPUResult Queue::SubmitCurrentCommandBuffer() {
  const VkPipelineStageFlags vk_wait_dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  VkSubmitInfo vk_submit_info{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 0u,
    .pWaitSemaphores = nullptr,
    .pWaitDstStageMask = &vk_wait_dst_stage_mask,
    .commandBufferCount = 1u,
    .pCommandBuffers = &m_vk_cmd_buffer,
    .signalSemaphoreCount = 0u,
    .pSignalSemaphores = nullptr
  };

  VkSemaphore vk_swap_chain_acquire_semaphore = m_vk_swap_chain_acquire_semaphore;
  if(vk_swap_chain_acquire_semaphore) {
    vk_submit_info.waitSemaphoreCount = 1u;
    vk_submit_info.pWaitSemaphores = &vk_swap_chain_acquire_semaphore;

    vk_submit_info.signalSemaphoreCount = 1u;
    vk_submit_info.pSignalSemaphores = &vk_swap_chain_acquire_semaphore;
  }

  MGPU_VK_FORWARD_ERROR(vkEndCommandBuffer(m_vk_cmd_buffer));
  MGPU_VK_FORWARD_ERROR(vkQueueSubmit(m_vk_queue, 1u, &vk_submit_info, m_vk_cmd_buffer_fence));
  m_fenced_cmd_buffers[m_current_cmd_buffer].submitted = true;
  m_fenced_cmd_buffers[m_current_cmd_buffer].timestamp_submitted = m_device->GetDeleterQueue().GetTimestamp();
  m_device->GetDeleterQueue().BumpTimestamp();
  m_current_cmd_buffer = (m_current_cmd_buffer + 1u) % m_fenced_cmd_buffers.size();
  return MGPU_SUCCESS;
}

MGPUResult Queue::BeginNextCommandBuffer() {
  FencedCommandBuffer& fenced_cmd_buffer = m_fenced_cmd_buffers[m_current_cmd_buffer];
  m_vk_cmd_buffer = fenced_cmd_buffer.vk_cmd_buffer;
  m_vk_cmd_buffer_fence = fenced_cmd_buffer.vk_fence;

  const VkCommandBufferBeginInfo vk_cmd_buffer_begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  if(fenced_cmd_buffer.submitted) {
    MGPU_VK_FORWARD_ERROR(vkWaitForFences(m_vk_device, 1u, &m_vk_cmd_buffer_fence, VK_TRUE, ~0ull));
    MGPU_VK_FORWARD_ERROR(vkResetFences(m_vk_device, 1u, &m_vk_cmd_buffer_fence));
    m_device->GetDeleterQueue().Drain(fenced_cmd_buffer.timestamp_submitted);
    fenced_cmd_buffer.submitted = false;
  }
  MGPU_VK_FORWARD_ERROR(vkResetCommandBuffer(m_vk_cmd_buffer, 0u));
  MGPU_VK_FORWARD_ERROR(vkBeginCommandBuffer(m_vk_cmd_buffer, &vk_cmd_buffer_begin_info));
  return MGPU_SUCCESS;
}

void Queue::HandleCmdBeginRenderPass(CommandListState& state, const BeginRenderPassCommand& command) {
  const bool have_depth_stencil_attachment = command.m_have_depth_stencil_attachment;
  const auto& depth_stencil_attachment = command.m_depth_stencil_attachment;

  RenderPassQuery render_pass_query{};

  for(size_t i = 0; i < command.m_color_attachments.Size(); i++) {
    const auto& color_attachment = command.m_color_attachments[i];
    if(color_attachment.texture_view != nullptr) {
      render_pass_query.SetColorAttachment(i, color_attachment.texture_view->Format(), color_attachment.load_op, color_attachment.store_op);
    }
  }

  if(have_depth_stencil_attachment) {
    render_pass_query.SetDepthStencilAttachment(
      depth_stencil_attachment.texture_view->Format(),
      depth_stencil_attachment.depth_load_op, depth_stencil_attachment.depth_store_op,
      depth_stencil_attachment.stencil_load_op, depth_stencil_attachment.stencil_store_op);
  }

  auto& pipeline_query = state.render_pass.pipeline_query;
  VkRenderPass vk_render_pass = m_render_pass_cache->GetRenderPass(render_pass_query).Unwrap(); // TODO(fleroviux): handle failure
  pipeline_query = {};
  pipeline_query.m_vk_render_pass = vk_render_pass;

  // Create a temporary framebuffer
  atom::Vector_N<VkImageView, limits::max_total_attachments> vk_attachment_image_views{};

  for(const auto& color_attachment : command.m_color_attachments) {
    const auto texture_view = (TextureView*)color_attachment.texture_view;
    if(texture_view != nullptr) {
      vk_attachment_image_views.PushBack(texture_view->Handle());
      state.render_pass.color_attachments.PushBack(texture_view);
    }
  }
  if(have_depth_stencil_attachment) {
    const auto texture_view = (TextureView*)depth_stencil_attachment.texture_view;
    vk_attachment_image_views.PushBack(texture_view->Handle());
    state.render_pass.depth_stencil_attachment = texture_view;
  }

  MGPUExtent3D texture_dimensions;

  if(have_depth_stencil_attachment) {
    texture_dimensions = depth_stencil_attachment.texture_view->GetTexture()->Extent();
  } else {
    for(const auto& color_attachment : command.m_color_attachments) {
      if(color_attachment.texture_view != nullptr) {
        texture_dimensions = color_attachment.texture_view->GetTexture()->Extent();
        break;
      }
    }
  }

  const VkFramebufferCreateInfo vk_framebuffer_create_info{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = vk_render_pass,
    .attachmentCount = (u32)vk_attachment_image_views.Size(),
    .pAttachments = vk_attachment_image_views.Data(),
    .width = texture_dimensions.width,
    .height = texture_dimensions.height,
    .layers = 1u
  };

  VkFramebuffer vk_framebuffer{};
  if(vkCreateFramebuffer(m_vk_device, &vk_framebuffer_create_info, nullptr, &vk_framebuffer) != VK_SUCCESS) {
    // TODO(fleroviux): report error to user
    ATOM_PANIC("failed to create VkFramebuffer");
  }

  // Begin the render pass
  atom::Vector_N<VkClearValue, limits::max_total_attachments> vk_clear_values{};
  for(const auto& color_attachment : command.m_color_attachments) {
    if(color_attachment.texture_view != nullptr) {
      // TODO(fleroviux): implement code paths for unsigned and signed integer texture formats
      vk_clear_values.PushBack({
        .color = {
          .float32 = {
            (f32)color_attachment.clear_color.r,
            (f32)color_attachment.clear_color.g,
            (f32)color_attachment.clear_color.b,
            (f32)color_attachment.clear_color.a
          }
        }
      });

      ((Texture*)color_attachment.texture_view->GetTexture())->TransitionState({
        .m_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .m_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .m_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
      }, m_vk_cmd_buffer);
    }
  }

  if(have_depth_stencil_attachment) {
    vk_clear_values.PushBack({
      .depthStencil = {
        .depth = depth_stencil_attachment.clear_depth,
        .stencil = depth_stencil_attachment.clear_stencil
      }
    });

    ((Texture*)depth_stencil_attachment.texture_view->GetTexture())->TransitionState({
      .m_image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .m_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      .m_pipeline_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
    }, m_vk_cmd_buffer);
  }

  const VkRenderPassBeginInfo vk_render_pass_begin_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = nullptr,
    .renderPass = vk_render_pass,
    .framebuffer = vk_framebuffer,
    .renderArea = {
      .offset = { .x = 0, .y = 0 },
      .extent = { .width = texture_dimensions.width, .height = texture_dimensions.height }
    },
    .clearValueCount = (u32)vk_clear_values.Size(),
    .pClearValues = vk_clear_values.Data()
  };
  vkCmdBeginRenderPass(m_vk_cmd_buffer, &vk_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

  // Set viewport and scissor test to sane defaults
  const VkViewport vk_viewport{
    .x = 0.f,
    .y = 0.f,
    .width = (f32)texture_dimensions.width,
    .height = (f32)texture_dimensions.height,
    .minDepth = 0.f,
    .maxDepth = 1.f
  };

  const VkRect2D vk_scissor{
    .offset = {.x = 0, .y = 0},
    .extent = {.width = 0x7FFFFFFF, .height = 0x7FFFFFFF}
  };

  vkCmdSetViewport(m_vk_cmd_buffer, 0u, 1u, &vk_viewport);
  vkCmdSetScissor(m_vk_cmd_buffer, 0u, 1u, &vk_scissor);

  // Destroy temporary framebuffer at the end of the frame.
  VkDevice vk_device = m_vk_device;
  m_deleter_queue->Schedule([vk_device, vk_framebuffer]() {
    vkDestroyFramebuffer(vk_device, vk_framebuffer, nullptr);
  });
}

void Queue::HandleCmdEndRenderPass(CommandListState& state) {
  vkCmdEndRenderPass(m_vk_cmd_buffer);

  state.render_pass = {};
}

void Queue::HandleCmdUseShaderProgram(CommandListState& state, const UseShaderProgramCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_shader_program != command.m_shader_program) {
    pipeline_query.m_shader_program = (ShaderProgram*)command.m_shader_program;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdUseRasterizerState(CommandListState& state, const UseRasterizerStateCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_rasterizer_state != command.m_rasterizer_state) {
    pipeline_query.m_rasterizer_state = (RasterizerState*)command.m_rasterizer_state;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdUseInputAssemblyState(CommandListState& state, const UseInputAssemblyStateCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_input_assembly_state != command.m_input_assembly_state) {
    pipeline_query.m_input_assembly_state = (InputAssemblyState*)command.m_input_assembly_state;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdUseColorBlendState(CommandListState& state, const UseColorBlendStateCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_color_blend_state != command.m_color_blend_state) {
    pipeline_query.m_color_blend_state = (ColorBlendState*)command.m_color_blend_state;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdUseVertexInputState(CommandListState& state, const UseVertexInputStateCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_vertex_input_state != command.m_vertex_input_state) {
    pipeline_query.m_vertex_input_state = (VertexInputState*)command.m_vertex_input_state;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdUseDepthStencilState(CommandListState& state, const UseDepthStencilStateCommand& command) {
  GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  if(pipeline_query.m_depth_stencil_state != command.m_depth_stencil_state) {
    pipeline_query.m_depth_stencil_state = (DepthStencilState*)command.m_depth_stencil_state;
    state.render_pass.require_pipeline_switch = true;
  }
}

void Queue::HandleCmdSetViewport(CommandListState& state, const SetViewportCommand& command) {
  const VkViewport vk_viewport{
    .x = command.m_viewport_x,
    .y = command.m_viewport_y,
    .width = command.m_viewport_width,
    .height = command.m_viewport_height,
    .minDepth = 0.f,
    .maxDepth = 1.f
  };

  (void)state;
  vkCmdSetViewport(m_vk_cmd_buffer, 0u, 1u, &vk_viewport);
}

void Queue::HandleCmdSetScissor(CommandListState& state, const SetScissorCommand& command) {
  const VkRect2D vk_scissor_rect{
    .offset = {
      .x = command.m_scissor_x,
      .y = command.m_scissor_y
    },
    .extent = {
      .width = command.m_scissor_width,
      .height = command.m_scissor_height
    }
  };

  (void)state;
  vkCmdSetScissor(m_vk_cmd_buffer, 0u, 1u, &vk_scissor_rect);
}

void Queue::HandleCmdBindVertexBuffer(CommandListState& state, const BindVertexBufferCommand& command) {
  const auto buffer = (Buffer*)command.m_buffer;
  // TODO(fleroviux): this breaks since we're inside of a render pass already. How to fix?
  //buffer->TransitionState({VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT}, m_vk_cmd_buffer);

  VkBuffer vk_buffer = buffer->Handle();
  vkCmdBindVertexBuffers(m_vk_cmd_buffer, command.m_binding, 1u, &vk_buffer, &command.m_buffer_offset);
}

void Queue::HandleCmdBindIndexBuffer(CommandListState& state, const BindIndexBufferCommand& command) {
  // TODO(fleroviux): implement a resource barrier
  vkCmdBindIndexBuffer(m_vk_cmd_buffer, ((Buffer*)command.m_buffer)->Handle(), command.m_buffer_offset, MGPUIndexFormatToVkIndexType(command.m_index_format));
}

void Queue::HandleCmdBindResourceSet(CommandListState& state, const BindResourceSetCommand& command) {
  const auto vk_pipeline_layout = state.render_pass.pipeline_query.m_shader_program->GetVkPipelineLayout();
  const auto vk_descriptor_set = ((ResourceSet*)command.m_resource_set)->Handle();
  // TODO(fleroviux): transition resources bound to the resource set to their required states
  vkCmdBindDescriptorSets(m_vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, command.m_index, 1u, &vk_descriptor_set, 0u, nullptr);
}

void Queue::HandleCmdDraw(CommandListState& state, const DrawCommand& command) {
  BindGraphicsPipelineForCurrentState(state);
  vkCmdDraw(m_vk_cmd_buffer, command.m_vertex_count, command.m_instance_count, command.m_first_vertex, command.m_first_instance);
}

void Queue::HandleCmdDrawIndexed(CommandListState& state, const DrawIndexedCommand& command) {
  BindGraphicsPipelineForCurrentState(state);
  vkCmdDrawIndexed(m_vk_cmd_buffer, command.m_index_count, command.m_instance_count, command.m_first_index, command.m_vertex_offset, command.m_first_instance);
}

void Queue::BindGraphicsPipelineForCurrentState(CommandListState& state) {
  if(!state.render_pass.require_pipeline_switch) {
    return;
  }
  state.render_pass.require_pipeline_switch = false;

  const GraphicsPipelineQuery& pipeline_query = state.render_pass.pipeline_query;

  // TODO(fleroviux): make sure that this can never happen in the future.
  bool pipeline_complete =
         pipeline_query.m_shader_program != nullptr
      && pipeline_query.m_rasterizer_state != nullptr
      && pipeline_query.m_input_assembly_state != nullptr
      && pipeline_query.m_color_blend_state != nullptr
      && pipeline_query.m_vertex_input_state != nullptr;

  if(!pipeline_complete) {
    return;
  }

  // TODO(fleroviux): handle failure to create the graphics pipeline.
  Result<VkPipeline> vk_pipeline_result = m_graphics_pipeline_cache.GetPipeline(pipeline_query);
  vkCmdBindPipeline(m_vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_result.Unwrap());
}

void Queue::DestroySwapChainAcquireSemaphore() {
  VkDevice vk_device = m_vk_device;
  VkSemaphore vk_semaphore = m_vk_swap_chain_acquire_semaphore;

  if(vk_semaphore) {
    m_deleter_queue->Schedule([vk_device, vk_semaphore]() {
      vkDestroySemaphore(vk_device, vk_semaphore, nullptr);
    });
    m_vk_swap_chain_acquire_semaphore = nullptr;
  }
}

}  // namespace mgpu::vulkan
