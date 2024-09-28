
#pragma once

#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "common/result.hpp"

namespace mgpu {

class TextureViewBase;

class TextureBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit TextureBase(const MGPUTextureCreateInfo& create_info) : m_create_info{create_info} {}

    virtual ~TextureBase() = default;

    [[nodiscard]] MGPUTextureType Type() const { return m_create_info.type; }
    [[nodiscard]] MGPUTextureFormat Format() const { return m_create_info.format; }
    [[nodiscard]] MGPUExtent3D Extent() const { return m_create_info.extent; }
    [[nodiscard]] u32 MipCount() const { return m_create_info.mip_count; }
    [[nodiscard]] u32 ArrayLayerCount() const { return m_create_info.array_layer_count; }

    [[nodiscard]] bool IsCubeCompatible() const {
      return m_create_info.type == MGPU_TEXTURE_TYPE_2D &&
             m_create_info.array_layer_count >= 6u &&
             m_create_info.extent.width == m_create_info.extent.height;
    }

    virtual Result<TextureViewBase*> CreateView(const MGPUTextureViewCreateInfo& create_info) = 0;

  private:
    MGPUTextureCreateInfo m_create_info;
};

}  // namespace mgpu
