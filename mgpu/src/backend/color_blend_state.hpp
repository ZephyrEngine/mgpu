
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class ColorBlendStateBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~ColorBlendStateBase() = default;
};

} // namespace mgpu
