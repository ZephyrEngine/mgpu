
#pragma once

#include <atom/integer.hpp>
#include <optional>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/vulkan/lib/vulkan_physical_device.hpp"
#include "backend/device.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Device final : public DeviceBase {
  public:
   ~Device() override;

    static Result<DeviceBase*> Create(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device);

    Result<BufferBase*> CreateBuffer(const MGPUBufferCreateInfo& create_info) override;

    [[nodiscard]] VkDevice Handle() {
      return m_vk_device;
    }

    [[nodiscard]] VmaAllocator GetVmaAllocator() {
      return m_vma_allocator;
    }

  private:
    struct QueueFamilyIndices {
      std::optional<u32> graphics_and_compute{};
      std::optional<u32> dedicated_compute{};
    };

    explicit Device(VkDevice vk_device, VmaAllocator vma_allocator);

    static QueueFamilyIndices SelectQueueFamilies(VulkanPhysicalDevice& vk_physical_device);
    static Result<VmaAllocator> CreateVmaAllocator(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, VkDevice vk_device);

    VkDevice m_vk_device{};
    VmaAllocator m_vma_allocator{};
};

}  // namespace mgpu::vulkan
