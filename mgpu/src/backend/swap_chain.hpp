
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class SwapChainBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SwapChainBase() = default;

    [[nodiscard]] virtual u32 GetNumberOfTextures() const = 0;
};

}  // namespace mgpu
