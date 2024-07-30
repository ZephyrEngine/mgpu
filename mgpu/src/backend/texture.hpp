
#pragma once

#include <mgpu/mgpu.h>

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

namespace mgpu {

class TextureBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit TextureBase(const MGPUTextureCreateInfo& create_info) : m_create_info{create_info} {}

    virtual ~TextureBase() = default;

  private:
    MGPUTextureCreateInfo m_create_info;
};

}  // namespace mgpu
