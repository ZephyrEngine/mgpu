
#include <mgpu/mgpu.h>
#include <cstring>
#include <limits>

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
  mgpu::Result<std::span<mgpu::PhysicalDeviceBase* const>> cxx_physical_devices_result = ((mgpu::InstanceBase*)instance)->EnumeratePhysicalDevices();

  MGPU_FORWARD_ERROR(cxx_physical_devices_result.Code());

  const std::span<mgpu::PhysicalDeviceBase* const> cxx_physical_devices = cxx_physical_devices_result.Unwrap();
  if(cxx_physical_devices.size() > std::numeric_limits<uint32_t>::max()) {
    return MGPU_INTERNAL_ERROR;
  }

  *physical_device_count = cxx_physical_devices.size();
  if(physical_devices != nullptr) {
    std::memcpy(physical_devices, cxx_physical_devices.data(), cxx_physical_devices.size_bytes());
  }

  return MGPU_SUCCESS;
}

MGPUResult mgpuInstanceCreateSurface(MGPUInstance instance, const MGPUSurfaceCreateInfo* create_info, MGPUSurface* surface) {
  mgpu::Result<mgpu::SurfaceBase*> cxx_surface_result = ((mgpu::InstanceBase*)instance)->CreateSurface(*create_info);
  MGPU_FORWARD_ERROR(cxx_surface_result.Code());
  *surface = (MGPUSurface)cxx_surface_result.Unwrap();
  return MGPU_SUCCESS;
}

void mgpuInstanceDestroy(MGPUInstance instance) {
  delete (mgpu::InstanceBase*)instance;
}

}  // extern "C"
