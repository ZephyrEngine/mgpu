
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
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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

RenderPassCache::~RenderPassCache() {
  for(const auto& [query_key, vk_render_pass] : m_query_key_to_vk_render_pass) {
    // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
    Device* device = m_device;
    device->GetDeleterQueue().Schedule([device, vk_render_pass]() {
      vkDestroyRenderPass(device->Handle(), vk_render_pass, nullptr);
    });
  }
}

Result<VkRenderPass> RenderPassCache::GetRenderPass(RenderPassQuery query) {
  const auto match = m_query_key_to_vk_render_pass.find(query.GetKey());
  if(match != m_query_key_to_vk_render_pass.end()) {
    return match->second;
  }

  for(size_t i = 0; i < m_vk_color_attachment_references.Size(); i++) {
    const auto [load_op, store_op] = query.GetColorAttachmentConfig(i);
    VkAttachmentDescription& vk_attachment_description = m_vk_attachment_descriptions[i];
    vk_attachment_description.loadOp = MGPULoadOpToVkAttachmentLoadOp(load_op);
    vk_attachment_description.storeOp = MGPUStoreOpToVkAttachmentStoreOp(store_op);
  }

  if(m_vk_subpass_description.pDepthStencilAttachment != nullptr) {
    VkAttachmentDescription& vk_attachment_description = m_vk_attachment_descriptions.Back();
    const auto [depth_load_op, depth_store_op] = query.GetDepthAttachmentConfig();
    const auto [stencil_load_op, stencil_store_op] = query.GetStencilAttachmentConfig();
    vk_attachment_description.loadOp = MGPULoadOpToVkAttachmentLoadOp(depth_load_op);
    vk_attachment_description.storeOp = MGPUStoreOpToVkAttachmentStoreOp(depth_store_op);
    vk_attachment_description.stencilLoadOp = MGPULoadOpToVkAttachmentLoadOp(stencil_load_op);
    vk_attachment_description.stencilStoreOp = MGPUStoreOpToVkAttachmentStoreOp(stencil_store_op);
  }

  VkRenderPass vk_render_pass{};
  MGPU_VK_FORWARD_ERROR(vkCreateRenderPass(m_device->Handle(), &m_vk_render_pass_create_info, nullptr, &vk_render_pass));
  m_query_key_to_vk_render_pass[query.GetKey()] = vk_render_pass;
  return vk_render_pass;
}

} // namespace mgpu::vulkan
