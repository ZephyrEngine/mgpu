
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
  const auto surface = (Surface*)create_info.surface;

  VkSwapchainCreateInfoKHR vk_swap_chain_create_info{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .surface = surface->Handle(),
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

  const auto old_swap_chain = surface->GetAssociatedSwapChain();
  if(old_swap_chain) {
    vk_swap_chain_create_info.oldSwapchain = ((SwapChain*)old_swap_chain)->Handle();
  }

  VkSwapchainKHR vk_swap_chain{};
  MGPU_VK_FORWARD_ERROR(vkCreateSwapchainKHR(device->Handle(), &vk_swap_chain_create_info, nullptr, &vk_swap_chain));
  return new SwapChain{device, vk_swap_chain, create_info};
}

Result<std::span<TextureBase* const>> SwapChain::EnumerateTextures() {
  return std::span<TextureBase* const>{m_textures};
}

Result<u32> SwapChain::AcquireNextTexture() {
  const VkSemaphoreCreateInfo vk_semaphore_create_info{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0
  };
  VkSemaphore vk_semaphore{};
  MGPU_VK_FORWARD_ERROR(vkCreateSemaphore(m_device->Handle(), &vk_semaphore_create_info, nullptr, &vk_semaphore));
  m_device->GetCommandQueue().SetSwapChainAcquireSemaphore(vk_semaphore);

  MGPU_VK_FORWARD_ERROR(vkAcquireNextImageKHR(m_device->Handle(), m_vk_swap_chain, 0ull, vk_semaphore, VK_NULL_HANDLE, &m_acquired_texture_index));
  return m_acquired_texture_index;
}

MGPUResult SwapChain::Present() {
  // TODO(fleroviux): use the queue the acquired swap chain texture is currently owned by (this needs to be implemented/tracked in the first place)
  // TODO(fleroviux): error out if no texture has been acquired
  return m_device->GetCommandQueue().Present(this, m_acquired_texture_index);
}

}  // namespace mgpu::vulkan
