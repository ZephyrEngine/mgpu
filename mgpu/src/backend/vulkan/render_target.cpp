
#include <atom/integer.hpp>
#include <atom/panic.hpp>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "render_target.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

RenderTarget::RenderTarget(
  Device* device,
  const atom::Vector_N<TextureView*, limits::max_total_attachments>& attachments,
  VkFramebuffer vk_framebuffer
)   : m_device{device}
    , m_attachments{attachments}
    , m_vk_framebuffer{vk_framebuffer} {
  const MGPUExtent3D extent = m_attachments[0]->GetTexture()->Extent();
  m_extent = {.width = extent.width, .height = extent.height};
}

RenderTarget::~RenderTarget() {
  // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkFramebuffer vk_framebuffer = m_vk_framebuffer;
  device->GetDeleterQueue().Schedule([device, vk_framebuffer]() {
    vkDestroyFramebuffer(device->Handle(), vk_framebuffer, nullptr);
  });
}

Result<RenderTargetBase*> RenderTarget::Create(Device* device, const MGPURenderTargetCreateInfo& create_info) {
  const std::span color_attachments{(TextureView* const*)create_info.color_attachments, create_info.color_attachment_count};
  const auto depth_stencil_attachment = (TextureView*)create_info.depth_stencil_attachment;

  MGPUExtent3D render_target_extent;

  if(depth_stencil_attachment != nullptr) {
    render_target_extent = depth_stencil_attachment->GetTexture()->Extent();
  } else {
    render_target_extent = color_attachments[0]->GetTexture()->Extent();
  }

  RenderPassQuery render_pass_query{};
  atom::Vector_N<TextureView*, limits::max_total_attachments> attachments{};
  atom::Vector_N<VkImageView, limits::max_total_attachments> attachment_vk_image_views{};

  for(size_t i = 0; i < color_attachments.size(); i++) {
    const auto color_attachment = color_attachments[i];
    attachments.PushBack(color_attachment);
    attachment_vk_image_views.PushBack(color_attachment->Handle());
    render_pass_query.SetColorAttachment(i, color_attachment->Format(), MGPU_LOAD_OP_DONT_CARE, MGPU_STORE_OP_DONT_CARE);
  }

  if(depth_stencil_attachment != nullptr) {
    attachments.PushBack(depth_stencil_attachment);
    attachment_vk_image_views.PushBack(depth_stencil_attachment->Handle());
    render_pass_query.SetDepthStencilAttachment(depth_stencil_attachment->Format(), MGPU_LOAD_OP_DONT_CARE, MGPU_STORE_OP_DONT_CARE, MGPU_LOAD_OP_DONT_CARE, MGPU_STORE_OP_DONT_CARE);
  }

  Result<VkRenderPass> vk_render_pass_result = device->GetRenderPassCache().GetRenderPass(render_pass_query);
  MGPU_FORWARD_ERROR(vk_render_pass_result.Code());

  VkRenderPass vk_render_pass = vk_render_pass_result.Unwrap();

  const VkFramebufferCreateInfo vk_framebuffer_create_info{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = vk_render_pass,
    .attachmentCount = (u32)attachment_vk_image_views.Size(),
    .pAttachments = attachment_vk_image_views.Data(),
    .width = render_target_extent.width,
    .height = render_target_extent.height,
    .layers = 1,
  };

  VkFramebuffer vk_framebuffer{};
  MGPU_VK_FORWARD_ERROR(vkCreateFramebuffer(device->Handle(), &vk_framebuffer_create_info, nullptr, &vk_framebuffer));
  return new RenderTarget{device, attachments, vk_framebuffer};
}

} // namespace mgpu::vulkan
