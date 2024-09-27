
#pragma once

#include <vulkan/vulkan.h>

#include "atom/result.hpp"
#include "backend/render_target.hpp"
#include "common/result.hpp"
#include "mgpu/mgpu.h"
#include "device.hpp"

namespace mgpu::vulkan {

class RenderTarget : public RenderTargetBase {
  public:
   ~RenderTarget() override;

    static Result<RenderTargetBase*> Create(Device* device, const MGPURenderTargetCreateInfo& create_info);

  private:
    RenderTarget(Device* device, VkFramebuffer vk_framebuffer, VkRenderPass vk_compatible_render_pass);

    Device* m_device;
    VkFramebuffer m_vk_framebuffer;
    VkRenderPass m_vk_compatible_render_pass;
};

} // namespace mgpu::vulkan