
#pragma once

#include <atom/integer.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/device.hpp"
#include "common/result.hpp"
#include "command_queue.hpp"
#include "deleter_queue.hpp"
#include "render_pass_cache.hpp"
#include "physical_device.hpp"

namespace mgpu::vulkan {

class Device final : public DeviceBase {
  public:
   ~Device() override;

    static Result<DeviceBase*> Create(
      VkInstance vk_instance,
      VulkanPhysicalDevice& vk_physical_device,
      const PhysicalDevice::QueueFamilyIndices& queue_family_indices,
      const MGPUPhysicalDeviceLimits& limits
    );

    [[nodiscard]] VkDevice Handle() { return m_vk_device; }
    [[nodiscard]] VmaAllocator GetVmaAllocator() { return m_vma_allocator; }
    [[nodiscard]] DeleterQueue& GetDeleterQueue() { return *m_deleter_queue; }
    [[nodiscard]] RenderPassCache& GetRenderPassCache() { return *m_render_pass_cache; }

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) override;
    Result<TextureBase*> CreateTexture(const MGPUTextureCreateInfo& create_info) override;
    Result<SwapChainBase*> CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) override;
    MGPUResult SubmitCommandList(const CommandList* command_list) override;
    MGPUResult Flush() override;

  private:
    Device(
      VkDevice vk_device,
      VmaAllocator vma_allocator,
      std::shared_ptr<DeleterQueue> deleter_queue,
      std::unique_ptr<CommandQueue> command_queue,
      std::shared_ptr<RenderPassCache> render_pass_cache,
      const MGPUPhysicalDeviceLimits& limits
    );

    static Result<VmaAllocator> CreateVmaAllocator(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, VkDevice vk_device);

    VkDevice m_vk_device;
    VmaAllocator m_vma_allocator;
    std::shared_ptr<DeleterQueue> m_deleter_queue;
    std::unique_ptr<CommandQueue> m_command_queue;
    std::shared_ptr<RenderPassCache> m_render_pass_cache;
};

}  // namespace mgpu::vulkan
