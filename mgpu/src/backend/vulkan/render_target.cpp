
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
  VkFramebuffer vk_framebuffer,
  VkRenderPass vk_compatible_render_pass
)   : m_device{device}
    , m_attachments{attachments}
    , m_vk_framebuffer{vk_framebuffer}
    , m_vk_compatible_render_pass{vk_compatible_render_pass} {
  const MGPUExtent3D extent = m_attachments[0]->GetTexture()->Extent();
  m_extent = {.width = extent.width, .height = extent.height};
}

RenderTarget::~RenderTarget() {
  // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkFramebuffer vk_framebuffer = m_vk_framebuffer;
  VkRenderPass vk_compatible_render_pass = m_vk_compatible_render_pass;
  device->GetDeleterQueue().Schedule([device, vk_framebuffer, vk_compatible_render_pass]() {
    vkDestroyFramebuffer(device->Handle(), vk_framebuffer, nullptr);
    vkDestroyRenderPass(device->Handle(), vk_compatible_render_pass, nullptr);
  });
}

Result<RenderTargetBase*> RenderTarget::Create(Device* device, const MGPURenderTargetCreateInfo& create_info) {
  atom::Vector_N<VkAttachmentDescription, limits::max_total_attachments> attachment_descriptions{};
  atom::Vector_N<VkAttachmentReference, limits::max_color_attachments> color_attachment_references{};
  VkAttachmentReference depth_stencil_attachment_reference{};

  const auto color_attachment_count = create_info.color_attachment_count;
  const auto color_attachments = (TextureView**)create_info.color_attachments;
  const auto depth_stencil_attachment = (TextureView*)create_info.depth_stencil_attachment;

  for(size_t i = 0; i < color_attachment_count; i++) {
    attachment_descriptions.PushBack({
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

    const VkAttachmentReference vk_attachment_reference{
      .attachment = (u32)i,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    color_attachment_references.PushBack(vk_attachment_reference);
  }

  if(depth_stencil_attachment) {
    attachment_descriptions.PushBack({
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

    depth_stencil_attachment_reference = VkAttachmentReference{
      .attachment = create_info.color_attachment_count,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
  }

  VkSubpassDescription vk_subpass_description{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = (u32)color_attachment_references.Size(),
    .pColorAttachments = color_attachment_references.Data(),
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = nullptr
  };

  if(depth_stencil_attachment != nullptr) {
    vk_subpass_description.pDepthStencilAttachment = &depth_stencil_attachment_reference;
  }

  const VkRenderPassCreateInfo vk_render_pass_create_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = (u32)attachment_descriptions.Size(),
    .pAttachments = attachment_descriptions.Data(),
    .subpassCount = 1,
    .pSubpasses = &vk_subpass_description,
    .dependencyCount = 0,
    .pDependencies = nullptr,
  };

  VkRenderPass vk_compatible_render_pass{};
  MGPU_VK_FORWARD_ERROR(vkCreateRenderPass(device->Handle(), &vk_render_pass_create_info, nullptr, &vk_compatible_render_pass));

  // ------------------------------------------------------- //

  MGPUExtent3D render_target_extent;

  if(depth_stencil_attachment != nullptr) {
    render_target_extent = depth_stencil_attachment->GetTexture()->Extent();
  } else {
    render_target_extent = color_attachments[0]->GetTexture()->Extent();
  }

  atom::Vector_N<TextureView*, limits::max_total_attachments> attachments{};
  atom::Vector_N<VkImageView, limits::max_total_attachments> attachment_vk_image_views{};

  for(size_t i = 0; i < color_attachment_count; i++) {
    attachments.PushBack(color_attachments[i]);
    attachment_vk_image_views.PushBack(color_attachments[i]->Handle());
  }

  if(depth_stencil_attachment != nullptr) {
    attachments.PushBack(depth_stencil_attachment);
    attachment_vk_image_views.PushBack(depth_stencil_attachment->Handle());
  }

  const VkFramebufferCreateInfo vk_framebuffer_create_info{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = vk_compatible_render_pass,
    .attachmentCount = (u32)attachment_vk_image_views.Size(),
    .pAttachments = attachment_vk_image_views.Data(),
    .width = render_target_extent.width,
    .height = render_target_extent.height,
    .layers = 1,
  };

  VkFramebuffer vk_framebuffer{};
  MGPU_VK_FORWARD_ERROR(vkCreateFramebuffer(device->Handle(), &vk_framebuffer_create_info, nullptr, &vk_framebuffer));
  return new RenderTarget{device, attachments, vk_framebuffer, vk_compatible_render_pass};
}

} // namespace mgpu::vulkan
