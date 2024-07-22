
#pragma once

#include <mgpu/mgpu.h>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "backend/buffer.hpp"

namespace mgpu {

class DeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~DeviceBase() = default;

    virtual Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) = 0;
};

} // namespace mgpu
