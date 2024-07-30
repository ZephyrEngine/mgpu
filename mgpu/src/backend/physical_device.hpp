
#pragma once

#include <mgpu/mgpu.h>

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "common/result.hpp"
#include "device.hpp"

namespace mgpu {

class PhysicalDeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit PhysicalDeviceBase(const MGPUPhysicalDeviceInfo& info) : m_info{info} {}

    virtual ~PhysicalDeviceBase() = default;

    [[nodiscard]] const MGPUPhysicalDeviceInfo& Info() const { return m_info; }
    [[nodiscard]] const MGPUPhysicalDeviceLimits& Limits() const { return m_info.limits; }

    virtual Result<DeviceBase*> CreateDevice() = 0;

  private:
    MGPUPhysicalDeviceInfo m_info{};
};

} // namespace mgpu
