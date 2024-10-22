
#pragma once

#include <mgpu/mgpu.h>
#include <atom/bit.hpp>
#include <atom/hash.hpp>
#include <atom/integer.hpp>
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/limits.hpp"
#include "common/result.hpp"
#include "deleter_queue.hpp"

namespace mgpu::vulkan {

struct RenderPassQuery {
  struct Hasher {
    size_t operator()(const RenderPassQuery& query) const;
  };

  void SetColorAttachment(size_t i, MGPUTextureFormat format, MGPULoadOp load_op, MGPUStoreOp store_op);

  void SetDepthStencilAttachment(
    MGPUTextureFormat format,
    MGPULoadOp depth_load_op,
    MGPUStoreOp depth_store_op,
    MGPULoadOp stencil_load_op,
    MGPUStoreOp stencil_store_op
  );

  [[nodiscard]] bool operator==(const RenderPassQuery& other_query) const;

  u32 m_color_attachment_set{};
  MGPUTextureFormat m_color_attachment_formats[limits::max_color_attachments];
  MGPULoadOp m_color_attachment_load_ops[limits::max_color_attachments];
  MGPUStoreOp m_color_attachment_store_ops[limits::max_color_attachments];

  bool m_have_depth_stencil_attachment{};
  MGPUTextureFormat m_depth_stencil_format;
  MGPULoadOp m_depth_load_op;
  MGPUStoreOp m_depth_store_op;
  MGPULoadOp m_stencil_load_op;
  MGPUStoreOp m_stencil_store_op;

  static_assert(limits::max_color_attachments <= atom::bit::number_of_bits<decltype(m_color_attachment_set)>());
};

class RenderPassCache {
  public:
    RenderPassCache(VkDevice vk_device, std::shared_ptr<DeleterQueue> deleter_queue);
   ~RenderPassCache();

    Result<VkRenderPass> GetRenderPass(const RenderPassQuery& query);

  private:
    VkDevice m_vk_device;
    std::shared_ptr<DeleterQueue> m_deleter_queue;
    std::unordered_map<RenderPassQuery, VkRenderPass, RenderPassQuery::Hasher> m_query_to_vk_render_pass{};
};

} // namespace mgpu::vulkan
