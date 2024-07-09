
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>
#include <memory>

#include "backend/render_device_backend_base.hpp"
#include "common/result.hpp"
#include "buffer.hpp"

namespace mgpu {

class RenderDevice {
  public:
    explicit RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend);

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo* create_info);
    Result<void*> MapBuffer(BufferBase* buffer);
    MGPUResult UnmapBuffer(BufferBase* buffer);
    MGPUResult FlushBuffer(BufferBase* buffer, u64 offset, u64 size);
    void DestroyBuffer(BufferBase* buffer);

  private:
    std::unique_ptr<RenderDeviceBackendBase> m_backend{};
};

}  // namespace mgpu