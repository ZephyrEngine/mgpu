
#include <atom/panic.hpp>
#include <cstring>

#include "device.hpp"
#include "physical_device.hpp"

namespace mgpu::vulkan {

PhysicalDevice::PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device)
    : m_vk_instance{vk_instance}
    , m_vk_physical_device{vk_physical_device} {
  PopulatePhysicalDeviceInfo();
}

MGPUResult PhysicalDevice::GetInfo(MGPUPhysicalDeviceInfo& physical_device_info) {
  physical_device_info = m_physical_device_info;
  return MGPU_SUCCESS;
}

Result<DeviceBase*> PhysicalDevice::CreateDevice() {
  return Device::Create(m_vk_instance, m_vk_physical_device);
}

void PhysicalDevice::PopulatePhysicalDeviceInfo() {
  static_assert(MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE <= VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);

  const VkPhysicalDeviceProperties& vk_physical_device_props = m_vk_physical_device.GetProperties();

  MGPUPhysicalDeviceType mgpu_physical_device_type;
  switch(vk_physical_device_props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;   break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;    break;
    default: ATOM_PANIC("unhandled Vulkan physical device type: {}", (int)vk_physical_device_props.deviceType);
  }

  std::strcpy(m_physical_device_info.device_name, vk_physical_device_props.deviceName);
  m_physical_device_info.device_type = mgpu_physical_device_type;

  // TODO(fleroviux): populate device limits with useful information.
}

}  // namespace mgpu::vulkan
