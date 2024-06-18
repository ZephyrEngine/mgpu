
#include "render_device.hpp"

namespace mgpu {

  RenderDevice::RenderDevice(std::unique_ptr<RenderDeviceBackendBase> backend)
      : m_backend{std::move(backend)} {
  }

}  // namespace mgpu