
#pragma once

#include <mgpu/mgpu.h>

#include <atom/panic.hpp>
#include <vulkan/vulkan.h>

#include "common/texture.hpp"

namespace mgpu::vulkan {

inline VkBufferUsageFlags MGPUBufferUsageToVkBufferUsage(MGPUBufferUsage buffer_usage) {
  VkBufferUsageFlags vk_buffer_usage{};
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_SRC)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_DST)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_UNIFORM_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_STORAGE_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDEX_BUFFER)    vk_buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_VERTEX_BUFFER)   vk_buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDIRECT_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  return vk_buffer_usage;
}

inline VkImageType MGPUTextureTypeToVkImageType(MGPUTextureType texture_type) {
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D: return VK_IMAGE_TYPE_1D;
    case MGPU_TEXTURE_TYPE_2D: return VK_IMAGE_TYPE_2D;
    case MGPU_TEXTURE_TYPE_3D: return VK_IMAGE_TYPE_3D;
    default: ATOM_PANIC("unhandled texture type: {}", (int)texture_type);
  }
}

inline VkFormat MGPUTextureFormatToVkFormat(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline VkImageUsageFlags MGPUTextureUsageToVkImageUsage(MGPUTextureFormat texture_format, MGPUTextureUsage texture_usage) {
  VkImageUsageFlags vk_image_usage{};
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_SRC) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_DST) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_SAMPLED)  vk_image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_STORAGE)  vk_image_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT) {
    if(MGPUTextureFormatToMGPUTextureAspect(texture_format) & MGPU_TEXTURE_ASPECT_COLOR) {
      vk_image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else {
      // TODO(fleroviux): validate that this always correct.
      vk_image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
  }
  return vk_image_usage;
}

inline VkImageUsageFlags VkImageUsageToMGPUTextureUsage(VkImageUsageFlags image_usage) {
  MGPUTextureUsage mgpu_texture_usage{};
  if(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_COPY_SRC;
  if(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_COPY_DST;
  if(image_usage & VK_IMAGE_USAGE_SAMPLED_BIT)      mgpu_texture_usage |= MGPU_TEXTURE_USAGE_SAMPLED;
  if(image_usage & VK_IMAGE_USAGE_STORAGE_BIT)      mgpu_texture_usage |= MGPU_TEXTURE_USAGE_STORAGE;
  if(image_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT;
  return mgpu_texture_usage;
}

inline VkImageAspectFlags MGPUTextureAspectToVkImageAspect(MGPUTextureAspect texture_aspect) {
  VkImageAspectFlags vk_image_aspect{};
  if(texture_aspect & MGPU_TEXTURE_ASPECT_COLOR)   vk_image_aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
  if(texture_aspect & MGPU_TEXTURE_ASPECT_DEPTH)   vk_image_aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
  if(texture_aspect & MGPU_TEXTURE_ASPECT_STENCIL) vk_image_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
  return vk_image_aspect;
}

inline VkImageViewType MGPUTextureViewTypeToVkImageViewType(MGPUTextureViewType texture_view_type) {
  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_1D:         return VK_IMAGE_VIEW_TYPE_1D;
    case MGPU_TEXTURE_VIEW_TYPE_2D:         return VK_IMAGE_VIEW_TYPE_2D;
    case MGPU_TEXTURE_VIEW_TYPE_3D:         return VK_IMAGE_VIEW_TYPE_3D;
    case MGPU_TEXTURE_VIEW_TYPE_CUBE:       return VK_IMAGE_VIEW_TYPE_CUBE;
    case MGPU_TEXTURE_VIEW_TYPE_1D_ARRAY:   return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY:   return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default: ATOM_PANIC("unhandled texture view type: {}", (int)texture_view_type);
  }
}

inline VkShaderStageFlagBits MGPUShaderStageBitToVkShaderStageBit(MGPUShaderStageBits shader_stage_bit) {
  switch(shader_stage_bit) {
    case MGPU_SHADER_STAGE_VERTEX:                  return VK_SHADER_STAGE_VERTEX_BIT;
    case MGPU_SHADER_STAGE_TESSELLATION_CONTROL:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case MGPU_SHADER_STAGE_TESSELLATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case MGPU_SHADER_STAGE_GEOMETRY:                return VK_SHADER_STAGE_GEOMETRY_BIT;
    case MGPU_SHADER_STAGE_FRAGMENT:                return VK_SHADER_STAGE_FRAGMENT_BIT;
    case MGPU_SHADER_STAGE_COMPUTE:                 return VK_SHADER_STAGE_COMPUTE_BIT;
    default: ATOM_PANIC("unhandled shader stage bit: {}", (int)shader_stage_bit);
  }
}

inline VkPolygonMode MGPUPolygonModeToVkPolygonMode(MGPUPolygonMode polygon_mode) {
  switch(polygon_mode) {
    case MGPU_POLYGON_MODE_FILL:  return VK_POLYGON_MODE_FILL;
    case MGPU_POLYGON_MODE_LINE:  return VK_POLYGON_MODE_LINE;
    default: ATOM_PANIC("unhandled polygon mode: {}", (int)polygon_mode);
  }
}

inline VkCullModeFlags MGPUCullModeToVkCullMode(MGPUCullMode cull_mode) {
  VkCullModeFlags vk_cull_mode{};
  if(cull_mode & MGPU_CULL_MODE_FRONT) vk_cull_mode |= VK_CULL_MODE_FRONT_BIT;
  if(cull_mode & MGPU_CULL_MODE_BACK)  vk_cull_mode |= VK_CULL_MODE_BACK_BIT;
  return vk_cull_mode;
}

inline VkFrontFace MGPUFrontFaceToVkFrontFace(MGPUFrontFace front_face) {
  switch(front_face) {
    case MGPU_FRONT_FACE_CLOCKWISE:         return VK_FRONT_FACE_CLOCKWISE;
    case MGPU_FRONT_FACE_COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default: ATOM_PANIC("unhandled front face: {}", (int)front_face);
  }
}

inline VkPrimitiveTopology MGPUPrimitiveTopologyToVkPrimitiveTopology(MGPUPrimitiveTopology topology) {
  switch(topology) {
    case MGPU_PRIMITIVE_TOPOLOGY_POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case MGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST:     return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default: ATOM_PANIC("unhandled primitive topology: {}", (int)topology);
  }
}

inline VkAttachmentLoadOp MGPULoadOpToVkAttachmentLoadOp(MGPULoadOp load_op) {
  switch(load_op) {
    case MGPU_LOAD_OP_CLEAR:     return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case MGPU_LOAD_OP_LOAD:      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case MGPU_LOAD_OP_DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default: ATOM_PANIC("unhandled load op: {}", (int)load_op);
  }
}

inline VkAttachmentStoreOp MGPUStoreOpToVkAttachmentStoreOp(MGPUStoreOp store_op) {
  switch(store_op) {
    case MGPU_STORE_OP_STORE:     return VK_ATTACHMENT_STORE_OP_STORE;
    case MGPU_STORE_OP_DONT_CARE: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default: ATOM_PANIC("unhandled store op: {}", (int)store_op);
  }
}

inline VkColorSpaceKHR MGPUColorSpaceToVkColorSpace(MGPUColorSpace color_space) {
  switch(color_space) {
    case MGPU_COLOR_SPACE_SRGB_NONLINEAR: return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    default: ATOM_PANIC("unhandled color space: {}", (int)color_space);
  }
}

inline VkPresentModeKHR MGPUPresentModeToVkPresentMode(MGPUPresentMode present_mode) {
  switch(present_mode) {
    case MGPU_PRESENT_MODE_IMMEDIATE:    return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case MGPU_PRESENT_MODE_MAILBOX:      return VK_PRESENT_MODE_MAILBOX_KHR;
    case MGPU_PRESENT_MODE_FIFO:         return VK_PRESENT_MODE_FIFO_KHR;
    case MGPU_PRESENT_MODE_FIFO_RELAXED: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    default: ATOM_PANIC("unhandled present mode: {}", (int)present_mode);
  }
}

}  // namespace mgpu::vulkan
