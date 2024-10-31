
#pragma once

#include <atom/integer.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/device.hpp"
#include "common/result.hpp"
#include "queue.hpp"
#include "deleter_queue.hpp"
#include "render_pass_cache.hpp"
#include "physical_device.hpp"

namespace mgpu::vulkan {

class Device final : public DeviceBase {
  public:
    struct Queues {
      std::unique_ptr<Queue> graphics_compute;
      std::unique_ptr<Queue> async_compute;
    };

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
    [[nodiscard]] Queue& GetCommandQueue() { return *m_queues.graphics_compute; } // TODO: remove this

    QueueBase* GetQueue(MGPUQueueType queue_type) override;
    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) override;
    Result<TextureBase*> CreateTexture(const MGPUTextureCreateInfo& create_info) override;
    Result<ShaderModuleBase*> CreateShaderModule(const u32* spirv_code, size_t spirv_byte_size) override;
    Result<ShaderProgramBase*> CreateShaderProgram(const MGPUShaderProgramCreateInfo& create_info) override;
    Result<SwapChainBase*> CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) override;

  private:
    Device(
      VkDevice vk_device,
      VmaAllocator vma_allocator,
      std::shared_ptr<DeleterQueue> deleter_queue,
      Queues&& queues,
      std::shared_ptr<RenderPassCache> render_pass_cache,
      const MGPUPhysicalDeviceLimits& limits
    );

    static Result<VmaAllocator> CreateVmaAllocator(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, VkDevice vk_device);

    VkDevice m_vk_device;
    VmaAllocator m_vma_allocator;
    std::shared_ptr<DeleterQueue> m_deleter_queue;
    Queues m_queues;
    std::shared_ptr<RenderPassCache> m_render_pass_cache;
};

}  // namespace mgpu::vulkan
