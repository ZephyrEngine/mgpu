
#include <vector>

#include "lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "device.hpp"
#include "resource_set_layout.hpp"

namespace mgpu::vulkan {

ResourceSetLayout::ResourceSetLayout(Device *device, VkDescriptorSetLayout vk_descriptor_set_layout)
    : m_device{device}
    , m_vk_descriptor_set_layout{vk_descriptor_set_layout} {
}

ResourceSetLayout::~ResourceSetLayout() {
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkDescriptorSetLayout vk_descriptor_set_layout = m_vk_descriptor_set_layout;
  device->GetDeleterQueue().Schedule([device, vk_descriptor_set_layout]() {
    vkDestroyDescriptorSetLayout(device->Handle(), vk_descriptor_set_layout, nullptr);
  });
}

Result<ResourceSetLayoutBase*> ResourceSetLayout::Create(Device* device, const MGPUResourceSetLayoutCreateInfo& create_info) {
  std::vector<VkDescriptorSetLayoutBinding> vk_bindings{};
  vk_bindings.resize(create_info.binding_count);

  for(size_t i = 0u; i < create_info.binding_count; i++) {
    const MGPUResourceSetLayoutBinding& mgpu_binding = create_info.bindings[i];

    vk_bindings.push_back({
      .binding = mgpu_binding.binding,
      .descriptorType = MGPUResourceBindingTypeToVkDescriptorType(mgpu_binding.type),
      .descriptorCount = 1u,
      .stageFlags = MGPUShaderStagesToVkShaderStageFlags(mgpu_binding.visibility),
      .pImmutableSamplers = nullptr
    });
  }

  const VkDescriptorSetLayoutCreateInfo vk_create_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0u,
    .bindingCount = create_info.binding_count,
    .pBindings = vk_bindings.data()
  };

  VkDescriptorSetLayout vk_descriptor_set_layout{};
  MGPU_VK_FORWARD_ERROR(vkCreateDescriptorSetLayout(device->Handle(), &vk_create_info, nullptr, &vk_descriptor_set_layout));
  return new ResourceSetLayout{device, vk_descriptor_set_layout};
}

} // namespace mgpu::vulkan
