
#include <mgpu/mgpu.h>

#include <atom/vector_n.hpp>
#include <vulkan/vulkan.h>

#include "backend/vertex_input_state.hpp"
#include "common/limits.hpp"

namespace mgpu::vulkan {

class VertexInputState : public VertexInputStateBase {
  public:
    explicit VertexInputState(const MGPUVertexInputStateCreateInfo& create_info);

    [[nodiscard]] const VkPipelineVertexInputStateCreateInfo& GetVkVertexInputState() const {
      return m_vk_vertex_input_state_create_info;
    }

  private:
    atom::Vector_N<VkVertexInputBindingDescription, limits::max_vertex_input_bindings> m_vk_vertex_input_bindings{};
    atom::Vector_N<VkVertexInputAttributeDescription, limits::max_vertex_input_attributes> m_vk_vertex_input_attributes{};
    VkPipelineVertexInputStateCreateInfo m_vk_vertex_input_state_create_info;
};

} // namespace mgpu::vulkan
