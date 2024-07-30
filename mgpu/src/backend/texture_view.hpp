
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class TextureViewBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~TextureViewBase() = default;
};

}  // namespace mgpu
