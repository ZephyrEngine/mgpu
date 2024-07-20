
#include <mgpu/mgpu.h>
#include <atom/panic.hpp>

#include "backend/vulkan/render_device_backend.hpp"
#include "frontend/render_device.hpp"

extern "C" {

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, SDL_Window* sdl_window, MGPURenderDevice* render_device) {
  mgpu::Result<std::unique_ptr<mgpu::RenderDeviceBackendBase>> render_device_backend{MGPU_BAD_ENUM};

  switch(backend) {
    case MGPU_BACKEND_VULKAN: render_device_backend = mgpu::RenderDeviceBackendVulkan::Create(sdl_window); break;
  }

  MGPU_FORWARD_ERROR(render_device_backend.Code());
  *render_device = (MGPURenderDevice)(new mgpu::RenderDevice{render_device_backend.Unwrap()});
  return MGPU_SUCCESS;
}

void mgpuDestroyRenderDevice(MGPURenderDevice render_device) {
  delete (mgpu::RenderDevice*)render_device;
}

MGPUResult mgpuCreateBuffer(MGPURenderDevice render_device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer) {
  mgpu::Result<mgpu::BufferBase*> buffer_result = ((mgpu::RenderDevice*)render_device)->CreateBuffer(create_info);
  MGPU_FORWARD_ERROR(buffer_result.Code());
  *buffer = (MGPUBuffer)buffer_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuMapBuffer(MGPURenderDevice render_device, MGPUBuffer buffer, void** address) {
  mgpu::Result<void*> address_result = ((mgpu::RenderDevice*)render_device)->MapBuffer((mgpu::BufferBase*)buffer);
  MGPU_FORWARD_ERROR(address_result.Code());
  *address = address_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuUnmapBuffer(MGPURenderDevice render_device, MGPUBuffer buffer) {
  return ((mgpu::RenderDevice*)render_device)->UnmapBuffer((mgpu::BufferBase*)buffer);
}

MGPUResult mgpuFlushBuffer(MGPURenderDevice render_device, MGPUBuffer buffer, uint64_t offset, uint64_t size) {
  return ((mgpu::RenderDevice*)render_device)->FlushBuffer((mgpu::BufferBase*)buffer, offset, size);
}

void mgpuDestroyBuffer(MGPURenderDevice render_device, MGPUBuffer buffer) {
  ((mgpu::RenderDevice*)render_device)->DestroyBuffer((mgpu::BufferBase*)buffer);
}

MGPUFence mgpuFenceSync(MGPURenderDevice render_device) {
  return ((mgpu::RenderDevice*)render_device)->FenceSync();
}

MGPUResult mgpuWaitFence(MGPURenderDevice render_device, MGPUFence fence) {
  return ((mgpu::RenderDevice*)render_device)->WaitFence(fence);
}

const char* mgpuResultCodeToString(MGPUResult result) {
  #define REGISTER(result_code) case result_code: return "" # result_code; break;

  switch(result) {
    REGISTER(MGPU_SUCCESS)
    REGISTER(MGPU_BAD_ENUM)
    REGISTER(MGPU_INTERNAL_ERROR)
    REGISTER(MGPU_OUT_OF_MEMORY)
    REGISTER(MGPU_BAD_DIMENSIONS)
    REGISTER(MGPU_BUFFER_NOT_HOST_VISIBLE)
    REGISTER(MGPU_BUFFER_NOT_MAPPED)
    default: ATOM_PANIC("internal error")
  }

  #undef REGISTER
}

}  // extern "C"
