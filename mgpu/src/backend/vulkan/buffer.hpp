
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/buffer.hpp"

namespace mgpu::vulkan {

class Buffer final : public BufferBase {
  public:
   ~Buffer() override;

    static Result<BufferBase*> Create(VkDevice vk_device, VmaAllocator vma_allocator, const MGPUBufferCreateInfo& create_info);

    [[nodiscard]] bool IsMapped() const override;

    Result<void*> Map() override;
    MGPUResult Unmap() override;
    MGPUResult FlushRange(u64 offset, u64 size) override;

  private:
    Buffer(VkDevice vk_device, VkBuffer vk_buffer, VmaAllocator vma_allocator, VmaAllocation vma_allocation, const MGPUBufferCreateInfo& create_info);

    VkDevice m_vk_device{};
    VkBuffer m_vk_buffer{};
    VmaAllocator m_vma_allocator{};
    VmaAllocation m_vma_allocation{};
    void* m_mapped_address{};
};

}  // namespace mgpu::vulkan
