
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/vector_n.hpp>
#include <span>
#include <unordered_map>
#include <utility>
#include <vulkan/vulkan.h>

#include "common/limits.hpp"
#include "common/result.hpp"
#include "device.hpp"
#include "texture_view.hpp"

namespace mgpu::vulkan {

class RenderPassQuery {
  public:
    using Key = u32;

    [[nodiscard]] Key GetKey() const { return m_query_key; }

    [[nodiscard]] std::pair<MGPULoadOp, MGPUStoreOp> GetColorAttachmentConfig(size_t attachment) const {
      return GetAttachmentConfig(attachment);
    }

    [[nodiscard]] std::pair<MGPULoadOp, MGPUStoreOp> GetDepthAttachmentConfig() const {
      return GetAttachmentConfig(depth_attachment);
    }

    [[nodiscard]] std::pair<MGPULoadOp, MGPUStoreOp> GetStencilAttachmentConfig() const {
      return GetAttachmentConfig(stencil_attachment);
    }

    void SetColorAttachmentConfig(size_t attachment, MGPULoadOp load_op, MGPUStoreOp store_op) {
      SetAttachmentConfig(attachment, load_op, store_op);
    }

    void SetDepthStencilAttachmentConfig(MGPULoadOp depth_load_op, MGPUStoreOp depth_store_op, MGPULoadOp stencil_load_op, MGPUStoreOp stencil_store_op) {
      SetAttachmentConfig(depth_attachment, depth_load_op, depth_store_op);
      SetAttachmentConfig(stencil_attachment, stencil_load_op, stencil_store_op);
    }

  private:
    static_assert(limits::max_color_attachments <= 8u);

    static constexpr int depth_attachment = 8;
    static constexpr int stencil_attachment = 9;

    [[nodiscard]] std::pair<MGPULoadOp, MGPUStoreOp> GetAttachmentConfig(size_t attachment) const {
      const size_t bit = attachment * 2;
      const auto load_op   = (MGPULoadOp)((m_query_key >> (bit + 1)) & 1u);
      const auto store_op = (MGPUStoreOp)((m_query_key >>  bit     ) & 1u);
      return std::make_pair(load_op, store_op);
    }

    void SetAttachmentConfig(size_t attachment, MGPULoadOp load_op, MGPUStoreOp store_op) {
      const size_t bit = attachment * 2;
      m_query_key &= ~3u << bit;
      m_query_key |= (load_op << 1 | store_op) << bit;
    }

    Key m_query_key{};
};

class RenderPassCache : atom::NonCopyable, atom::NonMoveable {
  public:
    RenderPassCache(Device* device, std::span<TextureView* const> color_attachments, TextureView* depth_stencil_attachment);
   ~RenderPassCache();

    Result<VkRenderPass> GetRenderPass(RenderPassQuery query);

  private:
    Device* m_device;
    atom::Vector_N<VkAttachmentDescription, limits::max_total_attachments> m_vk_attachment_descriptions{};
    atom::Vector_N<VkAttachmentReference, limits::max_color_attachments> m_vk_color_attachment_references{};
    VkAttachmentReference m_vk_depth_stencil_attachment_reference{};
    VkSubpassDescription m_vk_subpass_description{};
    VkRenderPassCreateInfo m_vk_render_pass_create_info{};
    std::unordered_map<RenderPassQuery::Key, VkRenderPass> m_query_key_to_vk_render_pass{};
};

} // namespace mgpu::vulkan
