
#pragma once

#include <mgpu/mgpu.h>

namespace mgpu {

class BufferBase {
  public:
    explicit BufferBase(const MGPUBufferCreateInfo& create_info) : m_create_info{create_info} {}

    virtual ~BufferBase() = default;

    const MGPUBufferCreateInfo& CreateInfo() {
      return m_create_info;
    }

  private:
    MGPUBufferCreateInfo m_create_info;
};

}  // namespace mgpu
