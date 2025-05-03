
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

    void Retire() {
      m_was_retired = true;
    }

    [[nodiscard]] bool WasRetired() const {
      return m_was_retired;
    }

    virtual Result<std::span<TextureBase* const>> EnumerateTextures() = 0;
    virtual Result<u32> AcquireNextTexture() = 0;
    virtual MGPUResult Present() = 0;

  private:
    bool m_was_retired{false};
};

}  // namespace mgpu
