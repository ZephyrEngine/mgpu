
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class VertexInputStateBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~VertexInputStateBase() = default;
};

} // namespace mgpu
