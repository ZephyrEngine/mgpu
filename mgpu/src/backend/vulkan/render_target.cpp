
#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <atom/vector_n.hpp>
#include <optional>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "render_target.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

RenderTarget::RenderTarget(Device* device, VkFramebuffer vk_framebuffer, VkRenderPass vk_compatible_render_pass)
    : m_device{device}
    , m_vk_framebuffer{vk_framebuffer}
    , m_vk_compatible_render_pass{vk_compatible_render_pass} {
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
  // TODO(fleroviux): handle hard limit in a better way, and clean this up in general

  atom::Vector_N<VkAttachmentDescription, 16> attachment_descriptions{};
  atom::Vector_N<VkAttachmentReference, 16> color_attachment_references{};
  std::optional<VkAttachmentReference> depth_stencil_attachment_reference{};

  for(size_t i = 0; i < create_info.attachment_count; i++) {
    auto attachment = (TextureView*)create_info.attachments[i];

    attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(attachment->Format()),
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

    if(attachment->Aspect() & MGPU_TEXTURE_ASPECT_COLOR) {
      color_attachment_references.PushBack(vk_attachment_reference);
    } else {
      depth_stencil_attachment_reference = vk_attachment_reference;
    }
  }

  const VkSubpassDescription vk_subpass_description{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = (u32)color_attachment_references.Size(),
    .pColorAttachments = color_attachment_references.Data(),
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = depth_stencil_attachment_reference.has_value() ? &depth_stencil_attachment_reference.value() : nullptr, // GROSS
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = nullptr
  };

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

  atom::Vector_N<VkImageView, 16> attachment_vk_image_views{};
  MGPUExtent3D extent{};

  for(size_t i = 0; i < create_info.attachment_count; i++) {
    auto attachment = (TextureView*)create_info.attachments[i];

    attachment_vk_image_views.PushBack(attachment->Handle());

    if(i == 0) {
      extent = attachment->GetTexture()->Extent();
    }
  }

  const VkFramebufferCreateInfo vk_framebuffer_create_info{
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .renderPass = vk_compatible_render_pass,
    .attachmentCount = (u32)attachment_vk_image_views.Size(),
    .pAttachments = attachment_vk_image_views.Data(),
    .width = extent.width,
    .height = extent.height,
    .layers = 1,
  };

  VkFramebuffer vk_framebuffer{};
  MGPU_VK_FORWARD_ERROR(vkCreateFramebuffer(device->Handle(), &vk_framebuffer_create_info, nullptr, &vk_framebuffer));
  return new RenderTarget{device, vk_framebuffer, vk_compatible_render_pass};
}

} // namespace mgpu::vulkan
