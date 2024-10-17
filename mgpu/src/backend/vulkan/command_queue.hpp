
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/vector_n.hpp>
#include <memory>
#include <vulkan/vulkan.h>

#include "backend/vulkan/physical_device.hpp"
#include "backend/command_list.hpp"
#include "common/result.hpp"
#include "common/limits.hpp"

namespace mgpu::vulkan {

class TextureView;

class CommandQueue : atom::NonCopyable, atom::NonMoveable {
  public:
   ~CommandQueue();

    static Result<std::unique_ptr<CommandQueue>> Create(VkDevice vk_device, const PhysicalDevice::QueueFamilyIndices& queue_family_indices);

    MGPUResult SubmitCommandList(const CommandList* command_list);
    MGPUResult Flush();

  private:
    CommandQueue(
      VkDevice vk_device,
      VkQueue vk_queue,
      VkCommandPool vk_cmd_pool,
      VkCommandBuffer vk_cmd_buffer,
      VkFence vk_cmd_buffer_fence
    );

    struct CommandListState {
      struct RenderPass {
        atom::Vector_N<TextureView*, limits::max_color_attachments> color_attachments{};
        TextureView* depth_stencil_attachment{};
      } render_pass{};
    };

    void HandleCmdBeginRenderPass(CommandListState& state, const BeginRenderPassCommand* command);
    void HandleCmdEndRenderPass(CommandListState& state);

    VkDevice m_vk_device;
    VkQueue m_vk_queue;
    VkCommandPool m_vk_cmd_pool;
    VkCommandBuffer m_vk_cmd_buffer;
    VkFence m_vk_cmd_buffer_fence;
};

}  // namespace mgpu::vulkan
