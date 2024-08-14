
#include <atom/integer.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#include "backend/swap_chain.hpp"
#include "common/result.hpp"
#include "device.hpp"
#include "texture.hpp"

namespace mgpu::vulkan {

class SwapChain final : public SwapChainBase {
  public:
   ~SwapChain() override;

    static Result<SwapChainBase*> Create(Device* device, const MGPUSwapChainCreateInfo& create_info);

    [[nodiscard]] VkSwapchainKHR Handle() { return m_vk_swap_chain; }

    [[nodiscard]] u32 GetNumberOfTextures() const override;

  private:
    SwapChain(Device* device, VkSwapchainKHR vk_swap_chain, const MGPUSwapChainCreateInfo& create_info);

    Device* m_device;
    VkSwapchainKHR m_vk_swap_chain;
    std::vector<TextureBase*> m_textures{};
};

}  // namespace mgpu::vulkan
