
#include <atom/panic.hpp>

#include "backend/vulkan/lib/vulkan_result.hpp"

#include "queue.hpp"
#include "shader_module.hpp"
#include "shader_program.hpp"
#include "swap_chain.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

// TODO(fleroviux): keep multiple command buffers around to a) avoid stalling and b) allow submission of multiple, smaller command buffers.

Queue::Queue(
  VkDevice vk_device,
  VkQueue vk_queue,
  VkCommandPool vk_cmd_pool,
  VkCommandBuffer vk_cmd_buffer,
  VkFence vk_cmd_buffer_fence,
  std::shared_ptr<DeleterQueue> deleter_queue,
  std::shared_ptr<RenderPassCache> render_pass_cache
)   : m_vk_device{vk_device}
    , m_vk_queue{vk_queue}
    , m_vk_cmd_pool{vk_cmd_pool}
    , m_vk_cmd_buffer{vk_cmd_buffer}
    , m_vk_cmd_buffer_fence{vk_cmd_buffer_fence}
    , m_deleter_queue{std::move(deleter_queue)}
    , m_render_pass_cache{std::move(render_pass_cache)} {
  const VkCommandBufferBeginInfo vk_cmd_buffer_begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  vkBeginCommandBuffer(m_vk_cmd_buffer, &vk_cmd_buffer_begin_info);
}

Queue::~Queue() {
  Flush();

  vkDestroyFence(m_vk_device, m_vk_cmd_buffer_fence, nullptr);
  vkFreeCommandBuffers(m_vk_device, m_vk_cmd_pool, 1u, &m_vk_cmd_buffer);
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
  MGPU_VK_FORWARD_ERROR(vkAllocateCommandBuffers(vk_device, &vk_cmd_buffer_alloc_info, &vk_cmd_buffer)); // TODO(fleroviux): this leaks memory

  const VkFenceCreateInfo vk_fence_create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0
  };
  MGPU_VK_FORWARD_ERROR(vkCreateFence(vk_device, &vk_fence_create_info, nullptr, &vk_cmd_buffer_fence)); // TODO(fleroviux): this leaks memory

  return std::unique_ptr<Queue>{new Queue{
    vk_device,
    vk_queue,
    vk_cmd_pool,
    vk_cmd_buffer,
    vk_cmd_buffer_fence,
    std::move(deleter_queue),
    std::move(render_pass_cache)
  }};
}

void Queue::SetSwapChainAcquireSemaphore(VkSemaphore vk_swap_chain_acquire_semaphore) {
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
  VkDevice vk_device = m_vk_device;
  m_deleter_queue->Schedule([vk_device, vk_swap_chain_acquire_semaphore]() {
    vkDestroySemaphore(vk_device,vk_swap_chain_acquire_semaphore, nullptr);
  });
  m_vk_swap_chain_acquire_semaphore = nullptr;

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
      case CommandType::Draw: HandleCmdDraw(state, *(DrawCommand*)command); break;
      default: {
        ATOM_PANIC("mgpu: Vulkan: unhandled command type: {}", (int)command_type);
      }
    }

    command = command->m_next;
  }

  return MGPU_SUCCESS;
}

MGPUResult Queue::Flush() {
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

  VkRenderPass vk_render_pass = m_render_pass_cache->GetRenderPass(render_pass_query).Unwrap(); // TODO(fleroviux): handle failure

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
            (float)color_attachment.clear_color.r,
            (float)color_attachment.clear_color.g,
            (float)color_attachment.clear_color.b,
            (float)color_attachment.clear_color.a
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
  if(state.render_pass.pipeline.shader_program != command.m_shader_program) {
    state.render_pass.pipeline.shader_program = command.m_shader_program;
    state.render_pass.pipeline.require_switch = true;
  }

  // TODO: redo this total hack

  const VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .setLayoutCount = 0u,
    .pSetLayouts = nullptr,
    .pushConstantRangeCount = 0u,
    .pPushConstantRanges = nullptr
  };
  VkPipelineLayout vk_pipeline_layout{};
  if(vkCreatePipelineLayout(m_vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
    ATOM_PANIC("failed to create pipeline layout?");
  }

  const std::span<const VkPipelineShaderStageCreateInfo> vk_shader_stages = ((ShaderProgram*)command.m_shader_program)->GetShaderStages();

  const VkPipelineVertexInputStateCreateInfo vk_vertex_input_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount = 0u,
    .pVertexBindingDescriptions = nullptr,
    .vertexAttributeDescriptionCount = 0u,
    .pVertexAttributeDescriptions = nullptr
  };

  const VkPipelineInputAssemblyStateCreateInfo vk_input_assembly_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  const VkViewport vk_viewport{
    .x = 0.f,
    .y = 0.f,
    .width = 1600.f,
    .height = 900.f,
    .minDepth = 0.f,
    .maxDepth = 1.f
  };

  const VkRect2D vk_scissor{
    .offset = {.x = 0, .y = 0},
    .extent = {.width = 0x7FFFFFFF, .height = 0x7FFFFFFF}
  };

  const VkPipelineViewportStateCreateInfo vk_viewport_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .viewportCount = 1u,
    .pViewports = &vk_viewport,
    .scissorCount = 1u,
    .pScissors = &vk_scissor
  };

  const VkPipelineRasterizationStateCreateInfo vk_rasterization_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_FRONT_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.f,
    .depthBiasClamp = 0.f,
    .depthBiasSlopeFactor = 0.f,
    .lineWidth = 1.f
  };

  const VkPipelineMultisampleStateCreateInfo vk_multisample_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 0.f,
    .pSampleMask = nullptr,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE
  };

  const VkPipelineDepthStencilStateCreateInfo vk_depth_stencil_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable = VK_FALSE,
    .depthWriteEnable = VK_FALSE,
    .depthCompareOp = VK_COMPARE_OP_ALWAYS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable = VK_FALSE,
    .front = {},
    .back = {},
    .minDepthBounds = 0.f,
    .maxDepthBounds = 1.f
  };

  const VkPipelineColorBlendAttachmentState vk_color_blend_attachment_state{
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = 0b1111
  };

  const VkPipelineColorBlendStateCreateInfo vk_color_blend_state_create_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_AND,
    .attachmentCount = 1u,
    .pAttachments = &vk_color_blend_attachment_state,
    .blendConstants = {0.f, 0.f, 0.f, 0.f}
  };

  RenderPassQuery render_pass_query{};
  render_pass_query.SetColorAttachment(0u, MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB, MGPU_LOAD_OP_DONT_CARE, MGPU_STORE_OP_DONT_CARE);

  Result<VkRenderPass> vk_render_pass_result = m_render_pass_cache->GetRenderPass(render_pass_query);
  VkRenderPass vk_render_pass = vk_render_pass_result.Unwrap(); // TODO: handle error

  const VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .stageCount = (u32)vk_shader_stages.size(),
    .pStages = vk_shader_stages.data(),
    .pVertexInputState = &vk_vertex_input_state_create_info,
    .pInputAssemblyState = &vk_input_assembly_state_create_info,
    .pTessellationState = nullptr,
    .pViewportState = &vk_viewport_state_create_info,
    .pRasterizationState = &vk_rasterization_state_create_info,
    .pMultisampleState = &vk_multisample_state_create_info,
    .pDepthStencilState = &vk_depth_stencil_state_create_info,
    .pColorBlendState = &vk_color_blend_state_create_info,
    .pDynamicState = nullptr,
    .layout = vk_pipeline_layout,
    .renderPass = vk_render_pass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = 0
  };

  VkPipeline vk_pipeline{};
  if(vkCreateGraphicsPipelines(m_vk_device, VK_NULL_HANDLE, 1u, &vk_graphics_pipeline_create_info, nullptr, &vk_pipeline) != VK_SUCCESS) {
    ATOM_PANIC("failed to create graphics pipeline?");
  }

  // TODO: at least fix the fucking memory leak

  vkCmdBindPipeline(m_vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);
}

void Queue::HandleCmdDraw(CommandListState& state, const DrawCommand& command) {
  vkCmdDraw(m_vk_cmd_buffer, command.m_vertex_count, command.m_instance_count, command.m_first_vertex, command.m_first_instance);
}

}  // namespace mgpu::vulkan
