
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class ShaderModuleBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~ShaderModuleBase() = default;
};

} // namespace mgpu
