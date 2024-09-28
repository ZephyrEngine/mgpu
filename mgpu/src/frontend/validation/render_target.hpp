
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>

#include "backend/texture_view.hpp"

inline MGPUResult validate_render_target_attachments(const MGPUPhysicalDeviceLimits& limits, const MGPURenderTargetCreateInfo& create_info) {
  const auto validate_attachment_texture_aspect = [](mgpu::TextureViewBase* attachment, MGPUTextureAspect valid_aspects) -> MGPUResult {
    if((attachment->Aspect() & valid_aspects) == 0u) {
      return MGPU_INCOMPATIBLE_TEXTURE_ASPECT;
    }
    return MGPU_SUCCESS;
  };

  const auto validate_attachment_texture_view_type = [](mgpu::TextureViewBase* attachment) -> MGPUResult {
    // TODO(fleroviux): allow MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY in the future?
    if(attachment->Type() != MGPU_TEXTURE_VIEW_TYPE_2D) {
      return MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE;
    }
    return MGPU_SUCCESS;
  };

  const auto validate_attachment_texture_usage = [](mgpu::TextureViewBase* attachment) -> MGPUResult {
    if((attachment->GetTexture()->Usage() & MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT) == 0u) {
      return MGPU_INCOMPATIBLE_TEXTURE_USAGE;
    }
    return MGPU_SUCCESS;
  };

  const u32 color_attachment_count = create_info.color_attachment_count;
  const auto color_attachments = (mgpu::TextureViewBase**)create_info.color_attachments;
  const auto depth_stencil_attachment = (mgpu::TextureViewBase*)create_info.depth_stencil_attachment;

  if(color_attachment_count == 0u && depth_stencil_attachment == nullptr) {
    return MGPU_BAD_DIMENSIONS;
  }

  if(color_attachment_count > limits.max_color_attachments) {
    return MGPU_BAD_DIMENSIONS;
  }

  MGPUExtent3D render_target_extent;

  if(depth_stencil_attachment != nullptr) {
    render_target_extent = depth_stencil_attachment->GetTexture()->Extent();
  } else {
    render_target_extent = color_attachments[0]->GetTexture()->Extent();
  }

  if(depth_stencil_attachment != nullptr) {
    // Render target extent is defined by the Depth/Stencil attachment (if present), so we do not need to validate the extent here.
    MGPU_FORWARD_ERROR(validate_attachment_texture_aspect(depth_stencil_attachment, MGPU_TEXTURE_ASPECT_DEPTH | MGPU_TEXTURE_ASPECT_STENCIL));
    MGPU_FORWARD_ERROR(validate_attachment_texture_view_type(depth_stencil_attachment));
    MGPU_FORWARD_ERROR(validate_attachment_texture_usage(depth_stencil_attachment));
  }

  for(size_t i = 0u; i < color_attachment_count; i++) {
    const auto color_attachment = color_attachments[i];

    MGPU_FORWARD_ERROR(validate_attachment_texture_aspect(color_attachment, MGPU_TEXTURE_ASPECT_COLOR));
    MGPU_FORWARD_ERROR(validate_attachment_texture_view_type(color_attachment));
    MGPU_FORWARD_ERROR(validate_attachment_texture_usage(color_attachment));

    MGPUExtent3D color_attachment_extent = color_attachment->GetTexture()->Extent();
    if(color_attachment_extent.width  != render_target_extent.width ||
       color_attachment_extent.height != render_target_extent.height) {
      return MGPU_BAD_DIMENSIONS;
    }
  }

  return MGPU_SUCCESS;
}