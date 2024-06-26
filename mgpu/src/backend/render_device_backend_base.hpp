
#pragma once

#include <mgpu/mgpu.h>

#include "frontend/buffer.hpp"
#include "common/result.hpp"

namespace mgpu {

class RenderDeviceBackendBase {
  public:
    virtual ~RenderDeviceBackendBase() = default;

    virtual Result<Buffer*> CreateBuffer(const MGPUBufferCreateInfo* create_info) = 0;
    virtual void DestroyBuffer(Buffer* buffer) = 0;
};

}  // namespace mgpu
