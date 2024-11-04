
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class InputAssemblyStateBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~InputAssemblyStateBase() = default;
};

} // namespace mgpu
