
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "render_pass_cache.hpp"

namespace mgpu::vulkan {

RenderPassCache::RenderPassCache(Device* device, std::span<TextureView* const> color_attachments, TextureView* depth_stencil_attachment)
    : m_device{device} {
  for(size_t i = 0; i < color_attachments.size(); i++) {
    m_vk_attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(color_attachments[i]->Format()),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });

    m_vk_color_attachment_references.PushBack({
      .attachment = (u32)i,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });
  }

  if(depth_stencil_attachment) {
    m_vk_attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(depth_stencil_attachment->Format()),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    });

    m_vk_depth_stencil_attachment_reference = VkAttachmentReference{
      .attachment = (u32)color_attachments.size(),
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
  }

  m_vk_subpass_description = {
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = (u32)m_vk_color_attachment_references.Size(),
    .pColorAttachments = m_vk_color_attachment_references.Data(),
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = nullptr
  };

  if(depth_stencil_attachment != nullptr) {
    m_vk_subpass_description.pDepthStencilAttachment = &m_vk_depth_stencil_attachment_reference;
  }

  m_vk_render_pass_create_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = (u32)m_vk_attachment_descriptions.Size(),
    .pAttachments = m_vk_attachment_descriptions.Data(),
    .subpassCount = 1,
    .pSubpasses = &m_vk_subpass_description,
    .dependencyCount = 0,
    .pDependencies = nullptr,
  };
}

RenderPassCache::~RenderPassCache() = default;

Result<VkRenderPass> RenderPassCache::GetRenderPassStub() {
  VkRenderPass vk_render_pass{};
  MGPU_VK_FORWARD_ERROR(vkCreateRenderPass(m_device->Handle(), &m_vk_render_pass_create_info, nullptr, &vk_render_pass));
  return vk_render_pass;
}

} // namespace mgpu::vulkan
