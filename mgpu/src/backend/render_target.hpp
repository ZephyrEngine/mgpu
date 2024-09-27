
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class RenderTargetBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~RenderTargetBase() = default;
};

} // namespace mgpu
