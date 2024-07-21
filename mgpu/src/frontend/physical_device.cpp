
#include <mgpu/mgpu.h>

#include "backend/physical_device.hpp"

extern "C" {

MGPUResult mgpuPhysicalDeviceGetInfo(MGPUPhysicalDevice physical_device, MGPUPhysicalDeviceInfo* physical_device_info) {
  return ((mgpu::PhysicalDeviceBase*)physical_device)->GetInfo(*physical_device_info);
}

MGPUResult mgpuPhysicalDeviceCreateDevice(MGPUPhysicalDevice physical_device, MGPUDevice* device) {
  mgpu::Result<mgpu::DeviceBase*> cxx_device_result = ((mgpu::PhysicalDeviceBase*)physical_device)->CreateDevice();

  MGPU_FORWARD_ERROR(cxx_device_result.Code());
  *device = (MGPUDevice)cxx_device_result.Unwrap();
  return MGPU_INTERNAL_ERROR;
}

}  // extern "C"