
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class ResourceSetBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~ResourceSetBase() = default;
};

} // namespace mgpu
