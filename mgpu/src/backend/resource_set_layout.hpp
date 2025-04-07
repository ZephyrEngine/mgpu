
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class ResourceSetLayoutBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~ResourceSetLayoutBase() = default;
};

} // namespace mgpu
