
#include <vulkan/vulkan.h>

#include "backend/swap_chain.hpp"
#include "common/result.hpp"
#include "device.hpp"

namespace mgpu::vulkan {

class SwapChain final : public SwapChainBase {
  public:
   ~SwapChain() override;

    static Result<SwapChainBase*> Create(Device* device, const MGPUSwapChainCreateInfo& create_info);

    [[nodiscard]] VkSwapchainKHR Handle() { return m_vk_swap_chain; }

  private:
    SwapChain(Device* device, VkSwapchainKHR vk_swap_chain);

    Device* m_device;
    VkSwapchainKHR m_vk_swap_chain;
};

}  // namespace mgpu::vulkan
