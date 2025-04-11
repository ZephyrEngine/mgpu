
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class SamplerBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SamplerBase() = default;
};

} // namespace mgpu
