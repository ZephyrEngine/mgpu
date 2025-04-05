
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/buffer.hpp"
#include "device.hpp"

namespace mgpu::vulkan {

class Buffer final : public BufferBase {
  public:
    struct State {
      VkAccessFlags m_access{VK_ACCESS_NONE};
      VkPipelineStageFlagBits m_pipeline_stages{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

      bool operator==(const State& other_state) const;
    };

   ~Buffer() override;

    static Result<BufferBase*> Create(Device* device, const MGPUBufferCreateInfo& create_info);

    [[nodiscard]] VkBuffer Handle() { return m_vk_buffer; }

    [[nodiscard]] bool IsMapped() const override;

    Result<void*> Map() override;
    MGPUResult Unmap() override;
    MGPUResult FlushRange(u64 offset, u64 size) override;

    void TransitionState(State new_state, VkCommandBuffer vk_command_buffer);

  private:
    Buffer(Device* device, VkBuffer vk_buffer, VmaAllocation vma_allocation, const MGPUBufferCreateInfo& create_info);

    Device* m_device{};
    VkBuffer m_vk_buffer{};
    VmaAllocation m_vma_allocation{};
    void* m_mapped_address{};
    State m_state{};
};

}  // namespace mgpu::vulkan
