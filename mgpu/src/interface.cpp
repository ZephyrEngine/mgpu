
#include <mgpu/mgpu.h>
#include <atom/panic.hpp>

#include "backend/opengl/render_device_backend.hpp"
#include "render_device.hpp"

extern "C" {

const char* mgpuResultCodeToString(MGPUResult result) {
  #define REGISTER(result_code) case result_code: return "" # result_code; break;

  switch(result) {
    REGISTER(MGPU_SUCCESS)
    REGISTER(MGPU_BAD_ENUM)
    default: ATOM_PANIC("internal error");
  }

  #undef REGISTER
}

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, MGPURenderDevice* render_device) {
  std::unique_ptr<mgpu::RenderDeviceBackendBase> render_device_backend{};

  switch(backend) {
    case MGPU_BACKEND_OPENGL: render_device_backend = std::make_unique<mgpu::OGLRenderDeviceBackend>(); break;
    case MGPU_BACKEND_VULKAN: ATOM_PANIC("internal error"); break;
    default: return MGPU_BAD_ENUM;
  }

  *render_device = (MGPURenderDevice)(new mgpu::RenderDevice{std::move(render_device_backend)});
  return MGPU_SUCCESS;
}

void mgpuDestroyRenderDevice(MGPURenderDevice render_device) {
  delete (mgpu::RenderDevice*)render_device;
}

}  // extern "C"