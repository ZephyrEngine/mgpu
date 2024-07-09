
#include <atom/panic.hpp>

#include "render_device.hpp"

namespace mgpu {

RenderDevice::RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend)
    : m_backend{std::move(backend)} {
}

Result<BufferBase*> RenderDevice::CreateBuffer(const MGPUBufferCreateInfo* create_info) {
  // TODO(fleroviux): error on empty usage set?
  if(create_info->size == 0u) {
    return MGPU_BAD_DIMENSIONS;
  }

  if(create_info->mapped_at_creation && !(create_info->flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE)) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }

  return m_backend->CreateBuffer(create_info);
}

Result<void*> RenderDevice::MapBuffer(BufferBase* buffer) {
  if(!(buffer->CreateInfo().flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE)) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }

  return m_backend->MapBuffer(buffer);
}

MGPUResult RenderDevice::UnmapBuffer(BufferBase* buffer) {
  if(!(buffer->CreateInfo().flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE)) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }

  return m_backend->UnmapBuffer(buffer);
}

MGPUResult RenderDevice::FlushBuffer(BufferBase* buffer, u64 offset, u64 size) {
  if(!(buffer->CreateInfo().flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE)) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }

  if(offset + size > buffer->CreateInfo().size) {
    return MGPU_BAD_DIMENSIONS;
  }

  return m_backend->FlushBuffer(buffer, offset, size);
}

void RenderDevice::DestroyBuffer(BufferBase* buffer) {
  m_backend->DestroyBuffer(buffer);
}

}  // namespace mgpu