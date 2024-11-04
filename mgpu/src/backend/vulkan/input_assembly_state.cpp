
#include "conversion.hpp"
#include "input_assembly_state.hpp"

namespace mgpu::vulkan {

InputAssemblyState::InputAssemblyState(const MGPUInputAssemblyStateCreateInfo& create_info) {
  m_vk_input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .topology = MGPUPrimitiveTopologyToVkPrimitiveTopology(create_info.topology),
    .primitiveRestartEnable = create_info.primitive_restart_enable
  };
}

} // namespace mgpu::vulkan
