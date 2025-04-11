
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/sampler.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device;

class Sampler : public SamplerBase {
  public:
   ~Sampler() override;

    static Result<SamplerBase*> Create(Device* device, const MGPUSamplerCreateInfo& create_info);

    [[nodiscard]] VkSampler Handle() { return m_vk_sampler; }

  private:
    Sampler(Device* device, VkSampler vk_sampler);

    Device* m_device;
    VkSampler m_vk_sampler;
};

} // namespace mgpu::vulkan
