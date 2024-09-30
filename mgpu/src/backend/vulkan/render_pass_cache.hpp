
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/vector_n.hpp>
#include <span>
#include <vulkan/vulkan.h>

#include "common/limits.hpp"
#include "common/result.hpp"
#include "device.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

class RenderPassCache : atom::NonCopyable, atom::NonMoveable {
  public:
    RenderPassCache(Device* device, std::span<TextureView*> color_attachments, TextureView* depth_stencil_attachment);
   ~RenderPassCache();

    Result<VkRenderPass> GetRenderPassStub();

  private:
    Device* m_device;
    atom::Vector_N<VkAttachmentDescription, limits::max_total_attachments> m_vk_attachment_descriptions{};
    atom::Vector_N<VkAttachmentReference, limits::max_color_attachments> m_vk_color_attachment_references{};
    VkAttachmentReference m_vk_depth_stencil_attachment_reference{};
    VkSubpassDescription m_vk_subpass_description{};
    VkRenderPassCreateInfo m_vk_render_pass_create_info{};
};

} // namespace mgpu::vulkan
