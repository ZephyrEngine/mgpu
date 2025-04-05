
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class DepthStencilStateBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~DepthStencilStateBase() = default;
};

} // namespace mgpu
