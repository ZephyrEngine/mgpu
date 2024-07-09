
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>

#include "frontend/buffer.hpp"
#include "common/result.hpp"

namespace mgpu {

class RenderDeviceBackendBase {
  public:
    virtual ~RenderDeviceBackendBase() = default;

    virtual Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo* create_info) = 0;
    virtual Result<void*> MapBuffer(BufferBase* buffer) = 0;
    virtual MGPUResult UnmapBuffer(BufferBase* buffer) = 0;
    virtual MGPUResult FlushBuffer(BufferBase* buffer, u64 offset, u64 size) = 0;
    virtual void DestroyBuffer(BufferBase* buffer) = 0;

    virtual MGPUFence FenceSync() = 0;
    virtual MGPUResult WaitFence(MGPUFence fence) = 0;
};

}  // namespace mgpu
