
#pragma once

#include <mgpu/mgpu.h>

#include "frontend/buffer.hpp"
#include "common/result.hpp"

namespace mgpu {

class RenderDeviceBackendBase {
  public:
    virtual ~RenderDeviceBackendBase() = default;

    virtual Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo* create_info) = 0;
    virtual void DestroyBuffer(BufferBase* buffer) = 0;
};

}  // namespace mgpu
