
#include "conversion.hpp"
#include "surface.hpp"
#include "swap_chain.hpp"

namespace mgpu::vulkan {

SwapChain::SwapChain(Device* device, VkSwapchainKHR vk_swap_chain)
    : m_device{device}
    , m_vk_swap_chain{vk_swap_chain} {
}

SwapChain::~SwapChain() {
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
    .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, // TODO
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
    .presentMode = VK_PRESENT_MODE_FIFO_KHR, // TODO
    .clipped = VK_TRUE,
    .oldSwapchain = nullptr
  };

  // TODO(fleroviux): investigate if this even works as intended
  if(create_info.old_swap_chain) {
    vk_swap_chain_create_info.oldSwapchain = ((SwapChain*)create_info.old_swap_chain)->Handle();
  }

  VkSwapchainKHR vk_swap_chain{};
  MGPU_VK_FORWARD_ERROR(vkCreateSwapchainKHR(device->Handle(), &vk_swap_chain_create_info, nullptr, &vk_swap_chain));
  return new SwapChain{device, vk_swap_chain};
}

}  // namespace mgpu::vulkan
