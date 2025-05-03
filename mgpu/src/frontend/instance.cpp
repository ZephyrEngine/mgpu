
#include <mgpu/mgpu.h>
#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

#include "backend/vulkan/instance.hpp"
#include "backend/physical_device.hpp"

extern "C" {

MGPUResult mgpuCreateInstance(MGPUBackendType backend_type, MGPUInstance* instance) {
  mgpu::Result<mgpu::InstanceBase*> cxx_instance_result{MGPU_BAD_ENUM};

  switch(backend_type) {
    case MGPU_BACKEND_TYPE_VULKAN: cxx_instance_result = mgpu::vulkan::Instance::Create(); break;
    default: break;
  }

  MGPU_FORWARD_ERROR(cxx_instance_result.Code());
  *instance = (MGPUInstance)cxx_instance_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuInstanceEnumeratePhysicalDevices(MGPUInstance instance, uint32_t* physical_device_count, MGPUPhysicalDevice* physical_devices) {
  const auto max_physical_devices = (size_t)*physical_device_count;

  mgpu::Result<std::span<mgpu::PhysicalDeviceBase* const>> cxx_physical_devices_result = ((mgpu::InstanceBase*)instance)->EnumeratePhysicalDevices();

  MGPU_FORWARD_ERROR(cxx_physical_devices_result.Code());

  const std::span<mgpu::PhysicalDeviceBase* const> cxx_physical_devices = cxx_physical_devices_result.Unwrap();
  if(cxx_physical_devices.size() > std::numeric_limits<uint32_t>::max()) {
    return MGPU_INTERNAL_ERROR;
  }

  *physical_device_count = cxx_physical_devices.size();
  if(physical_devices != nullptr) {
    const size_t copy_size = std::min(cxx_physical_devices.size(), max_physical_devices);
    std::memcpy(physical_devices, cxx_physical_devices.data(), copy_size * sizeof(MGPUPhysicalDevice));
    if(copy_size < cxx_physical_devices.size()) {
      return MGPU_INCOMPLETE;
    }
  }

  return MGPU_SUCCESS;
}

MGPUResult mgpuInstanceSelectPhysicalDevice(MGPUInstance instance, MGPUPowerPreference power_preference, MGPUPhysicalDevice* physical_device) {
  uint32_t mgpu_physical_device_count{};
  std::vector<MGPUPhysicalDevice> mgpu_physical_devices{};
  MGPU_FORWARD_ERROR(mgpuInstanceEnumeratePhysicalDevices(instance, &mgpu_physical_device_count, nullptr));
  mgpu_physical_devices.resize(mgpu_physical_device_count);
  MGPU_FORWARD_ERROR(mgpuInstanceEnumeratePhysicalDevices(instance, &mgpu_physical_device_count, mgpu_physical_devices.data()));

  std::optional<MGPUPhysicalDevice> discrete_gpu{};
  std::optional<MGPUPhysicalDevice> integrated_gpu{};
  std::optional<MGPUPhysicalDevice> virtual_gpu{};

  for(MGPUPhysicalDevice mgpu_physical_device : mgpu_physical_devices) {
    MGPUPhysicalDeviceInfo info{};
    MGPU_FORWARD_ERROR(mgpuPhysicalDeviceGetInfo(mgpu_physical_device, &info));
    switch(info.device_type) {
      case MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:     discrete_gpu = mgpu_physical_device; break;
      case MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: integrated_gpu = mgpu_physical_device; break;
      case MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       virtual_gpu = mgpu_physical_device; break;
    }
  }

  switch(power_preference) {
    case MGPU_POWER_PREFERENCE_LOW_POWER: {
      *physical_device = integrated_gpu.value_or(
        discrete_gpu.value_or(
          virtual_gpu.value_or((MGPUPhysicalDevice)MGPU_NULL_HANDLE)));
      break;
    }
    case MGPU_POWER_PREFERENCE_HIGH_PERFORMANCE: {
      *physical_device = discrete_gpu.value_or(
        integrated_gpu.value_or(
          virtual_gpu.value_or((MGPUPhysicalDevice)MGPU_NULL_HANDLE)));
      break;
    }
  }

  return MGPU_SUCCESS;
}

MGPUResult mgpuInstanceCreateSurface(MGPUInstance instance, const MGPUSurfaceCreateInfo* create_info, MGPUSurface* surface) {
  mgpu::Result<mgpu::SurfaceBase*> cxx_surface_result = ((mgpu::InstanceBase*)instance)->CreateSurface(*create_info);
  MGPU_FORWARD_ERROR(cxx_surface_result.Code());
  *surface = (MGPUSurface)cxx_surface_result.Unwrap();
  return MGPU_SUCCESS;
}

}  // extern "C"
