
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class CommandList : atom::NonCopyable, atom::NonMoveable {
  public:
    void Clear();
};

}  // namespace mgpu
