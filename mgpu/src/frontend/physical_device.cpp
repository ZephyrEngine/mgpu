
#include <mgpu/mgpu.h>

#include "backend/physical_device.hpp"

extern "C" {

MGPUResult mgpuPhysicalDeviceGetInfo(MGPUPhysicalDevice physical_device, MGPUPhysicalDeviceInfo* physical_device_info) {
  *physical_device_info = ((mgpu::PhysicalDeviceBase*)physical_device)->Info();
  return MGPU_SUCCESS;
}

MGPUResult mgpuPhysicalDeviceCreateDevice(MGPUPhysicalDevice physical_device, MGPUDevice* device) {
  mgpu::Result<mgpu::DeviceBase*> cxx_device_result = ((mgpu::PhysicalDeviceBase*)physical_device)->CreateDevice();

  MGPU_FORWARD_ERROR(cxx_device_result.Code());
  *device = (MGPUDevice)cxx_device_result.Unwrap();
  return MGPU_SUCCESS;
}

}  // extern "C"