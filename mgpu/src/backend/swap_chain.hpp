
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <span>

#include "backend/texture.hpp"
#include "common/result.hpp"

namespace mgpu {

class SwapChainBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SwapChainBase() = default;

    virtual Result<std::span<TextureBase* const>> EnumerateTextures() = 0;
    virtual Result<u32> AcquireNextTexture() = 0;
};

}  // namespace mgpu
