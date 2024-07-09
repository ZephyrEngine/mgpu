
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <memory>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/vulkan/utility/vulkan_command_pool.hpp"
#include "backend/vulkan/utility/vulkan_command_buffer.hpp"
#include "backend/vulkan/utility/vulkan_instance.hpp"
#include "backend/render_device_backend_base.hpp"
#include "common/result.hpp"

namespace mgpu {

class RenderDeviceBackendVulkan final : public RenderDeviceBackendBase {
  public:
    static Result<std::unique_ptr<RenderDeviceBackendBase>> Create(SDL_Window* sdl_window);

   ~RenderDeviceBackendVulkan() override;

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo* create_info) override;
    Result<void*> MapBuffer(BufferBase* buffer) override;
    MGPUResult UnmapBuffer(BufferBase* buffer) override;
    MGPUResult FlushBuffer(BufferBase* buffer, u64 offset, u64 size) override;
    void DestroyBuffer(BufferBase* buffer) override;

    MGPUFence FenceSync() override;
    MGPUResult WaitFence(MGPUFence fence) override;

  private:
    static Result<std::unique_ptr<VulkanInstance>> CreateVulkanInstance(SDL_Window* sdl_window);

    static Result<VkDevice> CreateVulkanDevice(
      const VulkanPhysicalDevice* vk_physical_device,
      u32& vk_graphics_compute_queue_family_index,
      std::vector<u32>& vk_present_queue_family_indices
    );

    static const VulkanPhysicalDevice* PickVulkanPhysicalDevice(const std::unique_ptr<VulkanInstance>& vk_instance);

    RenderDeviceBackendVulkan(
      std::unique_ptr<VulkanInstance> vk_instance,
      VkDevice vk_device,
      u32 vk_graphics_compute_queue_family_index,
      std::vector<u32>&& vk_present_queue_family_indices,
      VmaAllocator vma_allocator,
      VkSurfaceKHR vk_surface
    );

    std::unique_ptr<VulkanInstance> m_vk_instance;
    VkDevice m_vk_device;
    u32 m_vk_graphics_compute_queue_family_index;
    std::vector<u32> m_vk_present_queue_family_indices;
    VmaAllocator m_vma_allocator;
    VkSurfaceKHR m_vk_surface;

    std::shared_ptr<VulkanCommandPool> m_vk_command_pool{};
    std::shared_ptr<VulkanCommandBuffer> m_vk_command_buffer{};
};

}  // namespace mgpu
