
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "backend/texture.hpp"
#include "common/result.hpp"
#include "device.hpp"
#include "texture_state_tracker.hpp"

namespace mgpu::vulkan {

class Texture final : public TextureBase {
  public:
    using State = TextureState;

   ~Texture() override;

    static Result<TextureBase*> Create(Device* device, const MGPUTextureCreateInfo& create_info);
    static Texture* FromVkImage(Device* device, const MGPUTextureCreateInfo& create_info, VkImage vk_image);

    [[nodiscard]] VkImage Handle() { return m_vk_image; }

    Result<TextureViewBase*> CreateView(const MGPUTextureViewCreateInfo& create_info) override;

    void TransitionState(const TextureSubresourceState& texture_state_rect, VkCommandBuffer vk_command_buffer);

  private:
    Texture(Device* device, VkImage vk_image, VmaAllocation vma_allocation, const MGPUTextureCreateInfo& create_info);

    Device* m_device;
    VkImage m_vk_image;
    VmaAllocation m_vma_allocation;
    State m_state{
      .m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      .m_access = VK_ACCESS_NONE,
      .m_pipeline_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    };
    TextureStateTracker m_state_tracker;
};

}  // namespace mgpu::vulkan
