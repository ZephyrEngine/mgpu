
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class SwapChainBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SwapChainBase() = default;
};

}  // namespace mgpu
