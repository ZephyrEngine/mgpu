
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
    [[nodiscard]] u32 BaseMip() const { return m_create_info.base_mip; }
    [[nodiscard]] u32 MipCount() const { return m_create_info.mip_count; }
    [[nodiscard]] u32 BaseArrayLayer() const { return m_create_info.base_array_layer; }
    [[nodiscard]] u32 ArrayLayerCount() const { return m_create_info.array_layer_count; }

    virtual TextureBase* GetTexture() = 0;

  private:
    MGPUTextureViewCreateInfo m_create_info;
};

}  // namespace mgpu
