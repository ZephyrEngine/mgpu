
#include <atom/panic.hpp>

#include "instance.hpp"
#include "surface.hpp"

namespace mgpu::vulkan {

Instance::Instance(std::unique_ptr<VulkanInstance> vk_instance)
    : m_vk_instance{std::move(vk_instance)} {
  BuildPhysicalDeviceList();
}

Instance::~Instance() {
  for(auto physical_device : m_physical_devices) delete physical_device;
}

Result<InstanceBase*> Instance::Create() {
  const VkApplicationInfo app_info{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = "mgpu Vulkan driver",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "mgpu Vulkan driver",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_0
  };

  std::vector<const char*> vk_required_instance_extensions{"VK_KHR_surface"};
  std::vector<const char*> vk_required_instance_layers{};

#ifdef WIN32
  vk_required_instance_extensions.push_back("VK_KHR_win32_surface");
#endif

  // Enable validation layers in debug builds
#ifndef NDEBUG
  if(VulkanInstance::QueryInstanceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    vk_required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  Result<std::unique_ptr<VulkanInstance>> vk_instance_result = VulkanInstance::Create(
    app_info, vk_required_instance_extensions, vk_required_instance_layers, true);
  MGPU_FORWARD_ERROR(vk_instance_result.Code());

  return new Instance{vk_instance_result.Unwrap()};
}

Result<std::span<PhysicalDeviceBase* const>> Instance::EnumeratePhysicalDevices() {
  return std::span{(PhysicalDeviceBase* const*)m_physical_devices.data(), m_physical_devices.size()};
}

Result<SurfaceBase*> Instance::CreateSurface(const MGPUSurfaceCreateInfo& create_info) {
  return Surface::Create(m_vk_instance->Handle(), create_info);
}

void Instance::BuildPhysicalDeviceList() {
  for(auto& vk_physical_device : m_vk_instance->EnumeratePhysicalDevices()) {
    // We are only interested in GPUs, so we skip over any non-GPU devices (i.e. the CPU)
    VkPhysicalDeviceType vk_device_type = vk_physical_device->GetProperties().deviceType;
    if(vk_device_type != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
       vk_device_type != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
       vk_device_type != VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
      continue;
    }

    // We require VK_KHR_swapchain extensions for presentation
    if(!vk_physical_device->QueryDeviceExtensionSupport("VK_KHR_swapchain")) {
      continue;
    }

    m_physical_devices.push_back(new PhysicalDevice{m_vk_instance->Handle(), *vk_physical_device});
  }
}

}  // namespace mgpu::vulkan
