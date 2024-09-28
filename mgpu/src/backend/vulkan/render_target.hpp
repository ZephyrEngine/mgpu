
#pragma once

#include <atom/result.hpp>
#include <atom/vector_n.hpp>
#include <vulkan/vulkan.h>
#include <span>

#include "backend/render_target.hpp"
#include "common/limits.hpp"
#include "common/result.hpp"
#include "mgpu/mgpu.h"
#include "device.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

class TextureView;

class RenderTarget : public RenderTargetBase {
  public:
   ~RenderTarget() override;

    static Result<RenderTargetBase*> Create(Device* device, const MGPURenderTargetCreateInfo& create_info);

    [[nodiscard]] VkFramebuffer Handle() { return m_vk_framebuffer; }
    [[nodiscard]] VkRenderPass GetRenderPassStub() { return m_vk_compatible_render_pass; }
    [[nodiscard]] std::span<TextureView*> GetAttachments() { return m_attachments; }
    [[nodiscard]] MGPUExtent2D Extent() { return m_extent; }

  private:
    RenderTarget(
      Device* device,
      const atom::Vector_N<TextureView*, limits::max_total_attachments>& attachments,
      VkFramebuffer vk_framebuffer,
      VkRenderPass vk_compatible_render_pass
    );

    Device* m_device;
    atom::Vector_N<TextureView*, limits::max_total_attachments> m_attachments;
    MGPUExtent2D m_extent;
    VkFramebuffer m_vk_framebuffer;
    VkRenderPass m_vk_compatible_render_pass;
};

} // namespace mgpu::vulkan