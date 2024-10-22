
#include <atom/vector_n.hpp>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "device.hpp"
#include "render_pass_cache.hpp"

namespace mgpu::vulkan {

void RenderPassQuery::SetColorAttachment(size_t i, MGPUTextureFormat format, MGPULoadOp load_op, MGPUStoreOp store_op) {
  m_color_attachment_formats[i] = format;
  m_color_attachment_load_ops[i] = load_op;
  m_color_attachment_store_ops[i] = store_op;

  m_color_attachment_set |= 1 << i;
}

void RenderPassQuery::SetDepthStencilAttachment(MGPUTextureFormat format, MGPULoadOp depth_load_op, MGPUStoreOp depth_store_op, MGPULoadOp stencil_load_op, MGPUStoreOp stencil_store_op) {
  m_depth_stencil_format = format;
  m_depth_load_op = depth_load_op;
  m_depth_store_op = depth_store_op;
  m_stencil_load_op = stencil_load_op;
  m_stencil_store_op = stencil_store_op;

  m_have_depth_stencil_attachment = true;
}

[[nodiscard]] bool RenderPassQuery::operator==(const RenderPassQuery& other_query) const {
  if(m_color_attachment_set != other_query.m_color_attachment_set) {
    return false;
  }

  if(m_have_depth_stencil_attachment != other_query.m_have_depth_stencil_attachment) {
    return false;
  }

  u32 color_attachment_set = m_color_attachment_set;

  while(color_attachment_set != 0u) {
    const size_t i = __builtin_ctz(color_attachment_set);

    if(
      m_color_attachment_formats[i] != other_query.m_color_attachment_formats[i] ||
      m_color_attachment_load_ops[i] != other_query.m_color_attachment_load_ops[i] ||
      m_color_attachment_store_ops[i] != other_query.m_color_attachment_store_ops[i]
    ) {
      return false;
    }

    color_attachment_set ^= 1 << i;
  }

  if(m_have_depth_stencil_attachment) {
    if(
      m_depth_stencil_format != other_query.m_depth_stencil_format ||
      m_depth_load_op != other_query.m_depth_load_op ||
      m_depth_store_op != other_query.m_depth_store_op ||
      m_stencil_load_op != other_query.m_stencil_load_op ||
      m_stencil_store_op != other_query.m_stencil_store_op
    ) {
      return false;
    }
  }

  return true;
}

size_t RenderPassQuery::Hasher::operator()(const mgpu::vulkan::RenderPassQuery& query) const {
  size_t hash = 0u;

  u32 color_attachment_set = query.m_color_attachment_set;

  atom::hash_combine(hash, color_attachment_set);

  while(color_attachment_set != 0u) {
    const size_t i = __builtin_ctz(color_attachment_set);
    atom::hash_combine(hash, query.m_color_attachment_formats[i]);
    atom::hash_combine(hash, query.m_color_attachment_load_ops[i]);
    atom::hash_combine(hash, query.m_color_attachment_store_ops[i]);
    color_attachment_set ^= 1 << i;
  }

  const bool have_depth_stencil_attachment = query.m_have_depth_stencil_attachment;

  atom::hash_combine(hash, have_depth_stencil_attachment);

  if(have_depth_stencil_attachment) {
    atom::hash_combine(hash, query.m_depth_stencil_format);
    atom::hash_combine(hash, query.m_depth_load_op);
    atom::hash_combine(hash, query.m_depth_store_op);
    atom::hash_combine(hash, query.m_stencil_load_op);
    atom::hash_combine(hash, query.m_stencil_store_op);
  }

  return hash;
}

RenderPassCache::RenderPassCache(VkDevice vk_device, std::shared_ptr<DeleterQueue> deleter_queue)
    : m_vk_device{vk_device}
    , m_deleter_queue{std::move(deleter_queue)} {
}

RenderPassCache::~RenderPassCache() {
  for(const auto& [query_key, vk_render_pass] : m_query_to_vk_render_pass) {
    // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
    VkDevice vk_device = m_vk_device;
    m_deleter_queue->Schedule([vk_device, vk_render_pass]() {
      vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);
    });
  }
}

Result<VkRenderPass> RenderPassCache::GetRenderPass(const RenderPassQuery& query) {
  const auto match = m_query_to_vk_render_pass.find(query);
  if(match != m_query_to_vk_render_pass.end()) {
    return match->second;
  }

  atom::Vector_N<VkAttachmentDescription, limits::max_total_attachments> vk_attachment_descriptions{};
  atom::Vector_N<VkAttachmentReference, limits::max_color_attachments> vk_color_attachment_references{};
  VkAttachmentReference vk_depth_stencil_attachment_reference;

  size_t i = 0;
  u32 color_attachment_set = query.m_color_attachment_set;

  while(color_attachment_set != 0u) {
    if(color_attachment_set & (1 << i)) {
      vk_color_attachment_references.PushBack({
        .attachment = (u32)vk_attachment_descriptions.Size(),
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      });

      vk_attachment_descriptions.PushBack({
        .flags = 0,
        .format = MGPUTextureFormatToVkFormat(query.m_color_attachment_formats[i]),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = MGPULoadOpToVkAttachmentLoadOp(query.m_color_attachment_load_ops[i]),
        .storeOp = MGPUStoreOpToVkAttachmentStoreOp(query.m_color_attachment_store_ops[i]),
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      });

      color_attachment_set ^= 1 << i;
    } else {
      vk_color_attachment_references.PushBack({
        .attachment = VK_ATTACHMENT_UNUSED,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED
      });
    }

    i++;
  }

  const bool have_depth_stencil_attachment = query.m_have_depth_stencil_attachment;

  if(have_depth_stencil_attachment) {
    vk_attachment_descriptions.PushBack({
      .flags = 0,
      .format = MGPUTextureFormatToVkFormat(query.m_depth_stencil_format),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = MGPULoadOpToVkAttachmentLoadOp(query.m_depth_load_op),
      .storeOp = MGPUStoreOpToVkAttachmentStoreOp(query.m_depth_store_op),
      .stencilLoadOp = MGPULoadOpToVkAttachmentLoadOp(query.m_stencil_load_op),
      .stencilStoreOp = MGPUStoreOpToVkAttachmentStoreOp(query.m_stencil_store_op),
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    });

    vk_depth_stencil_attachment_reference = {
      .attachment = (u32)vk_color_attachment_references.Size(),
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
  }

  const VkSubpassDescription vk_subpass_description{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = nullptr,
    .colorAttachmentCount = (u32)vk_color_attachment_references.Size(),
    .pColorAttachments = vk_color_attachment_references.Data(),
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = have_depth_stencil_attachment ? &vk_depth_stencil_attachment_reference : nullptr,
    .preserveAttachmentCount = 0u,
    .pPreserveAttachments = nullptr
  };

  const VkRenderPassCreateInfo vk_render_pass_create_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = (u32)vk_attachment_descriptions.Size(),
    .pAttachments = vk_attachment_descriptions.Data(),
    .subpassCount = 1u,
    .pSubpasses = &vk_subpass_description,
    .dependencyCount = 0u,
    .pDependencies = nullptr
  };

  VkRenderPass vk_render_pass{};
  MGPU_VK_FORWARD_ERROR(vkCreateRenderPass(m_vk_device, &vk_render_pass_create_info, nullptr, &vk_render_pass));
  m_query_to_vk_render_pass[query] = vk_render_pass;
  return vk_render_pass;
}

} // namespace mgpu::vulkan
