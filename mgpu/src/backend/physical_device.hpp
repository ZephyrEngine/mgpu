
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "common/result.hpp"
#include "device.hpp"

namespace mgpu {

class PhysicalDeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~PhysicalDeviceBase() = default;

    virtual MGPUResult GetInfo(MGPUPhysicalDeviceInfo& physical_device_info) = 0;
    virtual Result<DeviceBase*> CreateDevice() = 0;
};

} // namespace mgpu
