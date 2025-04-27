
#pragma once

#include <atom/vector_n.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

#include "backend/command_list/command_list.hpp"
#include "backend/queue.hpp"
#include "common/result.hpp"
#include "common/limits.hpp"
#include "deleter_queue.hpp"
#include "graphics_pipeline_cache.hpp"
#include "render_pass_cache.hpp"

namespace mgpu::vulkan {

class Device;
class TextureView;
class ShaderProgram;
class RasterizerState;
class InputAssemblyState;
class ColorBlendState;
class VertexInputState;
class SwapChain;

class Queue final : public QueueBase {
  public:
   ~Queue() override;

    static Result<std::unique_ptr<Queue>> Create(
      VkDevice vk_device,
      u32 queue_family_index,
      std::shared_ptr<DeleterQueue> deleter_queue,
      std::shared_ptr<RenderPassCache> render_pass_cache
    );

    void SetDevice(Device* device);
    void SetSwapChainAcquireSemaphore(VkSemaphore vk_swap_chain_acquire_semaphore);
    MGPUResult Present(SwapChain* swap_chain, u32 texture_index);

    MGPUResult SubmitCommandList(const CommandList* command_list) override;
    MGPUResult BufferUpload(const BufferBase* buffer, std::span<const u8> data, u64 offset) override;
    MGPUResult TextureUpload(const TextureBase* texture, const MGPUTextureUploadRegion& region, const void* data) override;
    MGPUResult Flush() override;

  private:
    struct FencedCommandBuffer {
      VkCommandBuffer vk_cmd_buffer{};
      VkFence vk_fence{};
      bool submitted{false};
    };

    Queue(
      VkDevice vk_device,
      VkQueue vk_queue,
      VkCommandPool vk_cmd_pool,
      std::vector<FencedCommandBuffer> fenced_cmd_buffers,
      std::shared_ptr<DeleterQueue> deleter_queue,
      std::shared_ptr<RenderPassCache> render_pass_cache
    );

    struct CommandListState {
      struct RenderPass {
        atom::Vector_N<TextureView*, limits::max_color_attachments> color_attachments{};
        TextureView* depth_stencil_attachment{};
        GraphicsPipelineQuery pipeline_query{};
        bool require_pipeline_switch{true};
      } render_pass{};
    };

    MGPUResult SubmitCurrentCommandBuffer();
    MGPUResult BeginNextCommandBuffer();

    void HandleCmdBeginRenderPass(CommandListState& state, const BeginRenderPassCommand& command);
    void HandleCmdEndRenderPass(CommandListState& state);
    void HandleCmdUseShaderProgram(CommandListState& state, const UseShaderProgramCommand& command);
    void HandleCmdUseRasterizerState(CommandListState& state, const UseRasterizerStateCommand& command);
    void HandleCmdUseInputAssemblyState(CommandListState& state, const UseInputAssemblyStateCommand& command);
    void HandleCmdUseColorBlendState(CommandListState& state, const UseColorBlendStateCommand& command);
    void HandleCmdUseVertexInputState(CommandListState& state, const UseVertexInputStateCommand& command);
    void HandleCmdUseDepthStencilState(CommandListState& state, const UseDepthStencilStateCommand& command);
    void HandleCmdSetViewport(CommandListState& state, const SetViewportCommand& command);
    void HandleCmdSetScissor(CommandListState& state, const SetScissorCommand& command);
    void HandleCmdBindVertexBuffer(CommandListState& state, const BindVertexBufferCommand& command);
    void HandleCmdBindResourceSet(CommandListState& state, const BindResourceSetCommand& command);
    void HandleCmdDraw(CommandListState& state, const DrawCommand& command);

    void BindGraphicsPipelineForCurrentState(CommandListState& state);

    Device* m_device;
    VkDevice m_vk_device;
    VkQueue m_vk_queue;
    size_t m_current_cmd_buffer{};
    VkCommandPool m_vk_cmd_pool;
    VkCommandBuffer m_vk_cmd_buffer;
    VkFence m_vk_cmd_buffer_fence;
    std::vector<FencedCommandBuffer> m_fenced_cmd_buffers;

    std::shared_ptr<DeleterQueue> m_deleter_queue;
    std::shared_ptr<RenderPassCache> m_render_pass_cache;
    GraphicsPipelineCache m_graphics_pipeline_cache;
    VkSemaphore m_vk_swap_chain_acquire_semaphore{};
};

}  // namespace mgpu::vulkan
