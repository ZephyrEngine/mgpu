
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class BufferBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~BufferBase() = default;
};

} // namespace mgpu
