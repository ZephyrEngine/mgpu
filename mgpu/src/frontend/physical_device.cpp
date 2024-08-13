
#include <mgpu/mgpu.h>
#include <limits>

#include "backend/physical_device.hpp"

extern "C" {

MGPUResult mgpuPhysicalDeviceGetInfo(MGPUPhysicalDevice physical_device, MGPUPhysicalDeviceInfo* physical_device_info) {
  *physical_device_info = ((mgpu::PhysicalDeviceBase*)physical_device)->Info();
  return MGPU_SUCCESS;
}

MGPUResult mgpuPhysicalDeviceEnumerateSurfaceFormats(MGPUPhysicalDevice physical_device, MGPUSurface surface, uint32_t* surface_format_count, MGPUSurfaceFormat* surface_formats) {
  mgpu::Result<std::vector<MGPUSurfaceFormat>> surface_formats_result = ((mgpu::PhysicalDeviceBase*)physical_device)->EnumerateSurfaceFormats((mgpu::SurfaceBase*)surface);
  MGPU_FORWARD_ERROR(surface_formats_result.Code());

  std::vector<MGPUSurfaceFormat> surface_formats_vector = surface_formats_result.Unwrap();
  if(surface_formats_vector.size() > std::numeric_limits<uint32_t>::max()) {
    return MGPU_INTERNAL_ERROR;
  }

  *surface_format_count = surface_formats_vector.size();
  if(surface_formats != nullptr) {
    std::memcpy(surface_formats, surface_formats_vector.data(), surface_formats_vector.size() * sizeof(MGPUSurfaceFormat));
  }
  return MGPU_SUCCESS;
}

MGPUResult mgpuPhysicalDeviceEnumerateSurfacePresentModes(MGPUPhysicalDevice physical_device, MGPUSurface surface, uint32_t* present_mode_count, MGPUPresentMode* present_modes) {
  mgpu::Result<std::vector<MGPUPresentMode>> present_modes_result = ((mgpu::PhysicalDeviceBase*)physical_device)->EnumerateSurfacePresentModes((mgpu::SurfaceBase*)surface);
  MGPU_FORWARD_ERROR(present_modes_result.Code());

  std::vector<MGPUPresentMode> present_modes_vector = present_modes_result.Unwrap();
  if(present_modes_vector.size() > std::numeric_limits<uint32_t>::max()) {
    return MGPU_INTERNAL_ERROR;
  }

  *present_mode_count = present_modes_vector.size();
  if(present_modes != nullptr) {
    std::memcpy(present_modes, present_modes_vector.data(), present_modes_vector.size() * sizeof(MGPUPresentMode));
  }
  return MGPU_SUCCESS;
}

MGPUResult mgpuPhysicalDeviceCreateDevice(MGPUPhysicalDevice physical_device, MGPUDevice* device) {
  mgpu::Result<mgpu::DeviceBase*> cxx_device_result = ((mgpu::PhysicalDeviceBase*)physical_device)->CreateDevice();

  MGPU_FORWARD_ERROR(cxx_device_result.Code());
  *device = (MGPUDevice)cxx_device_result.Unwrap();
  return MGPU_SUCCESS;
}

}  // extern "C"