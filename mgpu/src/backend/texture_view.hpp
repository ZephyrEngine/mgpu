
#pragma once

#include <mgpu/mgpu.h>

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class TextureBase;

class TextureViewBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit TextureViewBase(const MGPUTextureViewCreateInfo& create_info) : m_create_info{create_info} {}

    virtual ~TextureViewBase() = default;

    [[nodiscard]] MGPUTextureViewType Type() const { return m_create_info.type; }
    [[nodiscard]] MGPUTextureFormat Format() const { return m_create_info.format; }
    [[nodiscard]] MGPUTextureAspect Aspect() const { return m_create_info.aspect; }

    virtual TextureBase* GetTexture() = 0;

  private:
    MGPUTextureViewCreateInfo m_create_info;
};

}  // namespace mgpu
