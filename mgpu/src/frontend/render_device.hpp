
#pragma once

#include <mgpu/mgpu.h>
#include <memory>

#include "backend/render_device_backend_base.hpp"
#include "common/result.hpp"
#include "buffer.hpp"

namespace mgpu {

class RenderDevice {
  public:
    explicit RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend);

    Result<Buffer*> CreateBuffer(const MGPUBufferCreateInfo* create_info);
    void DestroyBuffer(Buffer* buffer);

  private:
    std::unique_ptr<RenderDeviceBackendBase> m_backend{};
};

}  // namespace mgpu