
#pragma once

#include <memory>

#include "backend/render_device_backend_base.hpp"

namespace mgpu {

class RenderDevice {
  public:
    explicit RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend);

  private:
    std::unique_ptr<RenderDeviceBackendBase> m_backend{};
};

}  // namespace mgpu