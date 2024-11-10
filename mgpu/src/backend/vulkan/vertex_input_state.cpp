
#include "conversion.hpp"
#include "vertex_input_state.hpp"

namespace mgpu::vulkan {

VertexInputState::VertexInputState(const MGPUVertexInputStateCreateInfo& create_info) {
  for(size_t i = 0; i < create_info.binding_count; i++) {
    const MGPUVertexBinding& binding = create_info.bindings[i];

    m_vk_vertex_input_bindings.PushBack({
      .binding = binding.binding,
      .stride = binding.stride,
      .inputRate = MGPUVertexInputRateToVkVertexInputRate(binding.input_rate)
    });
  }

  for(size_t i = 0; i < create_info.attribute_count; i++) {
    const MGPUVertexAttribute& attribute = create_info.attributes[i];

    m_vk_vertex_input_attributes.PushBack({
      .location = attribute.location,
      .binding = attribute.binding,
      .format = MGPUVertexFormatToVkFormat(attribute.format),
      .offset = attribute.offset
    });
  }

  m_vk_vertex_input_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount = (u32)m_vk_vertex_input_bindings.Size(),
    .pVertexBindingDescriptions = m_vk_vertex_input_bindings.Data(),
    .vertexAttributeDescriptionCount = (u32)m_vk_vertex_input_attributes.Size(),
    .pVertexAttributeDescriptions = m_vk_vertex_input_attributes.Data()
  };
}

} // namespace mgpu::vulkan
