
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class RasterizerStateBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~RasterizerStateBase() = default;
};

} // namespace mgpu
