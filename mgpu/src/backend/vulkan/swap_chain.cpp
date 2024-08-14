
#include "conversion.hpp"
#include "surface.hpp"
#include "swap_chain.hpp"

namespace mgpu::vulkan {

SwapChain::SwapChain(Device* device, VkSwapchainKHR vk_swap_chain, const MGPUSwapChainCreateInfo& create_info)
    : m_device{device}
    , m_vk_swap_chain{vk_swap_chain} {
  u32 vk_image_count{};
  std::vector<VkImage> vk_images{};

  vkGetSwapchainImagesKHR(device->Handle(), vk_swap_chain, &vk_image_count, nullptr);
  vk_images.resize(vk_image_count);
  vkGetSwapchainImagesKHR(device->Handle(), vk_swap_chain, &vk_image_count, vk_images.data());

  const MGPUTextureCreateInfo texture_info{
    .format = create_info.format,
    .type = MGPU_TEXTURE_TYPE_2D,
    .extent = {
      .width = create_info.extent.width,
      .height = create_info.extent.height,
      .depth = 1u
    },
    .mip_count = 1u,
    .array_layer_count = 1u,
    .usage = create_info.usage
  };

  for(VkImage vk_image : vk_images) {
    m_textures.push_back(Texture::FromVkImage(device, texture_info, vk_image));
  }
}

SwapChain::~SwapChain() {
  for(auto texture : m_textures) delete texture;

  // Defer deletion of underlying Vulkan resources until the currently recorded frame has been fully processed on the GPU.
  // TODO(fleroviux): make this a little bit less verbose.
  Device* device = m_device;
  VkSwapchainKHR vk_swap_chain = m_vk_swap_chain;
  device->GetDeleterQueue().Schedule([device, vk_swap_chain]() {
    vkDestroySwapchainKHR(device->Handle(), vk_swap_chain, nullptr);
  });
}

Result<SwapChainBase*> SwapChain::Create(Device* device, const MGPUSwapChainCreateInfo& create_info) {
  VkSwapchainCreateInfoKHR vk_swap_chain_create_info{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .surface = ((Surface*)create_info.surface)->Handle(),
    .minImageCount = create_info.min_texture_count,
    .imageFormat = MGPUTextureFormatToVkFormat(create_info.format),
    .imageColorSpace = MGPUColorSpaceToVkColorSpace(create_info.color_space),
    .imageExtent = {
      .width = create_info.extent.width,
      .height = create_info.extent.height
    },
    .imageArrayLayers = 1u,
    .imageUsage = MGPUTextureUsageToVkImageUsage(create_info.format, create_info.usage),
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0u,
    .pQueueFamilyIndices = nullptr,
    .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = MGPUPresentModeToVkPresentMode(create_info.present_mode),
    .clipped = VK_TRUE,
    .oldSwapchain = nullptr
  };

  // TODO(fleroviux): investigate if this even works as intended
  if(create_info.old_swap_chain) {
    vk_swap_chain_create_info.oldSwapchain = ((SwapChain*)create_info.old_swap_chain)->Handle();
  }

  VkSwapchainKHR vk_swap_chain{};
  MGPU_VK_FORWARD_ERROR(vkCreateSwapchainKHR(device->Handle(), &vk_swap_chain_create_info, nullptr, &vk_swap_chain));
  return new SwapChain{device, vk_swap_chain, create_info};
}

}  // namespace mgpu::vulkan
