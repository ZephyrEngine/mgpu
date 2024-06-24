
#include <atom/panic.hpp>

#include "render_device.hpp"

namespace mgpu {

RenderDevice::RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend)
    : m_backend{std::move(backend)} {
}

Result<Buffer*> RenderDevice::CreateBuffer(const MGPUBufferCreateInfo* create_info) {
  // TODO(fleroviux): error on empty usage set?
  if(create_info->size == 0u) {
    return MGPU_BAD_DIMENSIONS;
  }
  return m_backend->CreateBuffer(create_info);
}

void RenderDevice::DestroyBuffer(Buffer* buffer) {
  m_backend->DestroyBuffer(buffer);
}

}  // namespace mgpu