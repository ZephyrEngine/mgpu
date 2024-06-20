
#include <mgpu/mgpu.h>
#include <atom/panic.hpp>

#include "backend/opengl/render_device_backend.hpp"
#include "backend/vulkan/render_device_backend.hpp"
#include "frontend/render_device.hpp"

extern "C" {

const char* mgpuResultCodeToString(MGPUResult result) {
  #define REGISTER(result_code) case result_code: return "" # result_code; break;

  switch(result) {
    REGISTER(MGPU_SUCCESS)
    REGISTER(MGPU_BAD_ENUM)
    REGISTER(MGPU_INTERNAL_ERROR)
    default: ATOM_PANIC("internal error");
  }

  #undef REGISTER
}

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, SDL_Window* sdl_window, MGPURenderDevice* render_device) {
  mgpu::Result<std::unique_ptr<mgpu::RenderDeviceBackendBase>> render_device_backend{MGPU_BAD_ENUM};

  switch(backend) {
    case MGPU_BACKEND_OPENGL: render_device_backend = mgpu::OGLRenderDeviceBackend::Create(sdl_window); break;
    case MGPU_BACKEND_VULKAN: render_device_backend = mgpu::VulkanRenderDeviceBackend::Create(sdl_window); break;
  }

  MGPU_FORWARD_ERROR(render_device_backend.Code());
  *render_device = (MGPURenderDevice)(new mgpu::RenderDevice{render_device_backend.Unwrap()});
  return MGPU_SUCCESS;
}

void mgpuDestroyRenderDevice(MGPURenderDevice render_device) {
  delete (mgpu::RenderDevice*)render_device;
}

}  // extern "C"