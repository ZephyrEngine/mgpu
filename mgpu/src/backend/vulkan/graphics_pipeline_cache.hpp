
#pragma once

#include <unordered_map>
#include <vulkan/vulkan.h>

#include "common/result.hpp"
#include "deleter_queue.hpp"

// TODO(fleroviux): invalidate cache entries when any associated pipelines are destroyed.

namespace mgpu::vulkan {

class ShaderProgram;
class RasterizerState;
class InputAssemblyState;
class ColorBlendState;
class VertexInputState;

struct GraphicsPipelineQuery {
  struct Hasher {
    size_t operator()(const GraphicsPipelineQuery& query) const;
  };

  const ShaderProgram* m_shader_program{};
  const RasterizerState* m_rasterizer_state{};
  const InputAssemblyState* m_input_assembly_state{};
  const ColorBlendState* m_color_blend_state{};
  const VertexInputState* m_vertex_input_state{};
  VkRenderPass m_vk_render_pass{};

  [[nodiscard]] bool operator==(const GraphicsPipelineQuery& other_query) const;
};

class GraphicsPipelineCache {
  public:
    GraphicsPipelineCache(VkDevice vk_device, std::shared_ptr<DeleterQueue> deleter_queue);
   ~GraphicsPipelineCache();

    Result<VkPipeline> GetPipeline(const GraphicsPipelineQuery& query);

  private:
    VkDevice m_vk_device;
    std::shared_ptr<DeleterQueue> m_deleter_queue;
    std::unordered_map<GraphicsPipelineQuery, VkPipeline, GraphicsPipelineQuery::Hasher> m_query_to_vk_pipeline{};
};

} // namespace mgpu::vulkan
