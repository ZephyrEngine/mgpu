
#pragma once

#include <atom/integer.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/device.hpp"
#include "common/result.hpp"
#include "deleter_queue.hpp"
#include "physical_device.hpp"

namespace mgpu::vulkan {

class Device final : public DeviceBase {
  public:
   ~Device() override;

    static Result<DeviceBase*> Create(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device, const MGPUPhysicalDeviceLimits& limits);

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) override;
    Result<TextureBase*> CreateTexture(const MGPUTextureCreateInfo& create_info) override;

    [[nodiscard]] VkDevice Handle() { return m_vk_device; }
    [[nodiscard]] VmaAllocator GetVmaAllocator() { return m_vma_allocator; }
    [[nodiscard]] DeleterQueue& GetDeleterQueue() { return m_deleter_queue; }

  private:
    struct QueueFamilyIndices {
      std::optional<u32> graphics_and_compute{};
      std::optional<u32> dedicated_compute{};
    };

    explicit Device(VkDevice vk_device, VmaAllocator vma_allocator, const MGPUPhysicalDeviceLimits& limits);

    static QueueFamilyIndices SelectQueueFamilies(VulkanPhysicalDevice& vk_physical_device);
    static Result<VmaAllocator> CreateVmaAllocator(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, VkDevice vk_device);

    VkDevice m_vk_device{};
    VmaAllocator m_vma_allocator{};
    DeleterQueue m_deleter_queue{};
};

}  // namespace mgpu::vulkan
