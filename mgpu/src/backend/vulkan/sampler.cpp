
#include "lib/vulkan_result.hpp"
#include "conversion.hpp"
#include "device.hpp"
#include "sampler.hpp"

namespace mgpu::vulkan {

Sampler::Sampler(Device* device, VkSampler vk_sampler)
    : m_device{device}
    , m_vk_sampler{vk_sampler} {
}

Sampler::~Sampler() {
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkSampler vk_sampler = m_vk_sampler;
  device->GetDeleterQueue().Schedule([device, vk_sampler]() {
    vkDestroySampler(device->Handle(), vk_sampler, nullptr);
  });
}

Result<SamplerBase*> Sampler::Create(Device* device, const MGPUSamplerCreateInfo& create_info) {
  const VkSamplerCreateInfo vk_create_info{
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .magFilter = MGPUTextureFilterToVkFilter(create_info.mag_filter),
    .minFilter = MGPUTextureFilterToVkFilter(create_info.min_filter),
    .mipmapMode = MGPUTextureFilterToVkSamplerMipmapMode(create_info.mip_filter),
    .addressModeU = MGPUSamplerAddressModeToVkSamplerAddressMode(create_info.address_mode_u),
    .addressModeV = MGPUSamplerAddressModeToVkSamplerAddressMode(create_info.address_mode_v),
    .addressModeW = MGPUSamplerAddressModeToVkSamplerAddressMode(create_info.address_mode_w),
    .mipLodBias = create_info.mip_lod_bias,
    .anisotropyEnable = create_info.anisotropy_enable,
    .maxAnisotropy = create_info.max_anisotropy,
    .compareEnable = create_info.compare_enable,
    .compareOp = MGPUCompareOpToVkCompareOp(create_info.compare_op),
    .minLod = create_info.min_lod,
    .maxLod = create_info.max_lod,
    .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE
  };
  VkSampler vk_sampler{};
  MGPU_VK_FORWARD_ERROR(vkCreateSampler(device->Handle(), &vk_create_info, nullptr, &vk_sampler));
  return new Sampler{device, vk_sampler};
}

} // namespace mgpu::vulkan
