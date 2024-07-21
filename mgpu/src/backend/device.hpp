
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class DeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~DeviceBase() = default;
};

} // namespace mgpu
