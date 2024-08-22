
#pragma once

#include <mgpu/mgpu.h>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "backend/buffer.hpp"
#include "backend/command_list.hpp"
#include "backend/swap_chain.hpp"
#include "backend/texture.hpp"

namespace mgpu {

class DeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit DeviceBase(const MGPUPhysicalDeviceLimits& limits) : m_limits{limits} {}

    virtual ~DeviceBase() = default;

    [[nodiscard]] const MGPUPhysicalDeviceLimits& Limits() const { return m_limits; }

    virtual Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) = 0;
    virtual Result<TextureBase*> CreateTexture(const MGPUTextureCreateInfo& create_info) = 0;
    virtual Result<SwapChainBase*> CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) = 0;
    virtual MGPUResult SubmitCommandList(CommandList* command_list) = 0;

  private:
    MGPUPhysicalDeviceLimits m_limits{};
};

} // namespace mgpu
