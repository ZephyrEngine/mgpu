
#pragma once

#include "atom/non_copyable.hpp"
#include "atom/non_moveable.hpp"

namespace mgpu {

class ShaderProgramBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~ShaderProgramBase() = default;
};

} // namespace mgpu
