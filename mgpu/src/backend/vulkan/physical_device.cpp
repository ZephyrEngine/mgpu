
#include <atom/panic.hpp>
#include <cstring>

#include "device.hpp"
#include "physical_device.hpp"

namespace mgpu::vulkan {

PhysicalDevice::PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device, const QueueFamilyIndices& queue_family_indices)
    : PhysicalDeviceBase{GetInfo(vk_physical_device)}
    , m_vk_instance{vk_instance}
    , m_vk_physical_device{vk_physical_device}
    , m_queue_family_indices{queue_family_indices} {
}

Result<DeviceBase*> PhysicalDevice::CreateDevice() {
  return Device::Create(m_vk_instance, m_vk_physical_device, m_queue_family_indices, Limits());
}

MGPUPhysicalDeviceInfo PhysicalDevice::GetInfo(VulkanPhysicalDevice& vk_physical_device) {
  static_assert(MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE <= VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);

  const VkPhysicalDeviceProperties& vk_device_props = vk_physical_device.GetProperties();
  MGPUPhysicalDeviceInfo mgpu_device_info{};

  MGPUPhysicalDeviceType mgpu_physical_device_type;
  switch(vk_device_props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;   break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;    break;
    default: ATOM_PANIC("unhandled Vulkan physical device type: {}", (int)vk_device_props.deviceType);
  }

  std::strcpy(mgpu_device_info.device_name, vk_device_props.deviceName);
  mgpu_device_info.device_type = mgpu_physical_device_type;
  mgpu_device_info.limits = GetLimits(vk_device_props.limits);
  return mgpu_device_info;
}

MGPUPhysicalDeviceLimits PhysicalDevice::GetLimits(const VkPhysicalDeviceLimits& vk_device_limits) {
  MGPUPhysicalDeviceLimits mgpu_device_limits{};

  // TODO(fleroviux): check if in practice any devices have lower cube texture limit than 2D texture dimension limit.
  mgpu_device_limits.max_texture_dimension_1d = vk_device_limits.maxImageDimension1D;
  mgpu_device_limits.max_texture_dimension_2d = std::min(vk_device_limits.maxImageDimension2D, vk_device_limits.maxImageDimensionCube);
  mgpu_device_limits.max_texture_dimension_3d = vk_device_limits.maxImageDimension3D;
  mgpu_device_limits.max_texture_array_layers = vk_device_limits.maxImageArrayLayers;
  return mgpu_device_limits;
}

}  // namespace mgpu::vulkan
