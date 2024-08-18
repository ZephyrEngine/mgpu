
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class SurfaceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SurfaceBase() = default;
};

}  // namespace mgpu
