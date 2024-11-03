
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class RasterizerStateBase {
  public:
    virtual ~RasterizerStateBase() = default;
};

} // namespace mgpu
