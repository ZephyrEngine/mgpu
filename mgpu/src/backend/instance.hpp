
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <span>

#include "common/result.hpp"
#include "physical_device.hpp"
#include "surface.hpp"

namespace mgpu {

class InstanceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~InstanceBase() = default;

    virtual Result<std::span<PhysicalDeviceBase* const>> EnumeratePhysicalDevices() = 0;
    virtual Result<SurfaceBase*> CreateSurface(const MGPUSurfaceCreateInfo& create_info) = 0;
};

} // namespace mgpu
