
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "common/result.hpp"

namespace mgpu {

class BufferBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit BufferBase(const MGPUBufferCreateInfo& create_info) : m_create_info{create_info} {}

    virtual ~BufferBase() = default;

    [[nodiscard]] u64  Size() const { return m_create_info.size; }
    [[nodiscard]] bool HostVisible() const { return m_create_info.flags & MGPU_BUFFER_FLAGS_HOST_VISIBLE; }
    [[nodiscard]] virtual bool IsMapped() const = 0;

    virtual Result<void*> Map() = 0;
    virtual MGPUResult Unmap() = 0;
    virtual MGPUResult FlushRange(u64 offset, u64 size) = 0;

  private:
    MGPUBufferCreateInfo m_create_info{};
};

} // namespace mgpu
