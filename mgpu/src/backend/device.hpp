
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "common/result.hpp"

namespace mgpu {

class QueueBase;
class BufferBase;
class TextureBase;
class ResourceSetLayoutBase;
class ResourceSetBase;
class ShaderModuleBase;
class ShaderProgramBase;
class RasterizerStateBase;
class InputAssemblyStateBase;
class ColorBlendStateBase;
class VertexInputStateBase;
class DepthStencilStateBase;
class SwapChainBase;

class DeviceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit DeviceBase(const MGPUPhysicalDeviceLimits& limits) : m_limits{limits} {}

    virtual ~DeviceBase() = default;

    [[nodiscard]] const MGPUPhysicalDeviceLimits& Limits() const { return m_limits; }

    virtual QueueBase* GetQueue(MGPUQueueType queue_type) = 0;
    virtual Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) = 0;
    virtual Result<TextureBase*> CreateTexture(const MGPUTextureCreateInfo& create_info) = 0;
    virtual Result<ResourceSetLayoutBase*> CreateResourceSetLayout(const MGPUResourceSetLayoutCreateInfo& create_info) = 0;
    virtual Result<ResourceSetBase*> CreateResourceSet(const MGPUResourceSetCreateInfo& create_info) = 0;
    virtual Result<ShaderModuleBase*> CreateShaderModule(const u32* spirv_code, size_t spirv_byte_size) = 0;
    virtual Result<ShaderProgramBase*> CreateShaderProgram(const MGPUShaderProgramCreateInfo& create_info) = 0;
    virtual Result<RasterizerStateBase*> CreateRasterizerState(const MGPURasterizerStateCreateInfo& create_info) = 0;
    virtual Result<InputAssemblyStateBase*> CreateInputAssemblyState(const MGPUInputAssemblyStateCreateInfo& create_info) = 0;
    virtual Result<ColorBlendStateBase*> CreateColorBlendState(const MGPUColorBlendStateCreateInfo& create_info) = 0;
    virtual Result<VertexInputStateBase*> CreateVertexInputState(const MGPUVertexInputStateCreateInfo& create_info) = 0;
    virtual Result<DepthStencilStateBase*> CreateDepthStencilState(const MGPUDepthStencilStateCreateInfo& create_info) = 0;
    virtual Result<SwapChainBase*> CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) = 0;

  private:
    MGPUPhysicalDeviceLimits m_limits{};
};

} // namespace mgpu
