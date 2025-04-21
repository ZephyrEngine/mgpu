
#pragma once

#include <mgpu/mgpu.h>

#include <atom/float.hpp>
#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/panic.hpp>
#include <atom/vector_n.hpp>
#include <utility>
#include <vector>

#include "backend/device.hpp"
#include "common/bump_allocator.hpp"
#include "commands.hpp"

namespace mgpu {

class CommandList : atom::NonCopyable, atom::NonMoveable {
  public:
    explicit CommandList(DeviceBase* device) : m_device{device} {
      m_memory_chunks.emplace_back(k_chunk_size);
      Clear();
    }

    [[nodiscard]] bool HasErrors() const { return m_state.has_errors || IsIncomplete(); }
    [[nodiscard]] bool IsIncomplete() const { return m_state.inside_render_pass; }

    [[nodiscard]] const CommandBase* GetListHead() const { return m_head; }

    void Clear() {
      m_memory_chunks[0].Reset();
      m_active_chunk = 0u;
      m_head = nullptr;
      m_tail = nullptr;
      m_state = {};
    }

    RenderCommandEncoder* CmdBeginRenderPass(const MGPURenderPassBeginInfo& begin_info) {
      if(m_state.inside_render_pass) {
        m_state.has_errors = true;
      }
      m_state.inside_render_pass = true;

      const auto& command = Push<BeginRenderPassCommand>(this, begin_info);

      RenderCommandEncoder& encoder = command.m_render_command_encoder;
      encoder.CmdUseRasterizerState(m_device->GetDefaultRasterizerState());
      encoder.CmdUseColorBlendState(m_device->GetDefaultColorBlendState(begin_info.color_attachment_count));
      encoder.CmdUseInputAssemblyState(m_device->GetDefaultInputAssemblyState());
      encoder.CmdUseVertexInputState(m_device->GetDefaultVertexInputState());
      encoder.CmdUseDepthStencilState(m_device->GetDefaultDepthStencilState());
      return &encoder;
    }

    void CmdEndRenderPass() {
      m_state.inside_render_pass = false;
      Push<EndRenderPassCommand>();
    }

    void CmdUseShaderProgram(ShaderProgramBase* shader_program) {
      Push<UseShaderProgramCommand>(shader_program);
    }

    void CmdUseRasterizerState(RasterizerStateBase* rasterizer_state) {
      Push<UseRasterizerStateCommand>(rasterizer_state);
    }

    void CmdUseInputAssemblyState(InputAssemblyStateBase* input_assembly_state) {
      Push<UseInputAssemblyStateCommand>(input_assembly_state);
    }

    void CmdUseColorBlendState(ColorBlendStateBase* color_blend_state) {
      Push<UseColorBlendStateCommand>(color_blend_state);
    }

    void CmdUseVertexInputState(VertexInputStateBase* vertex_input_state) {
      Push<UseVertexInputStateCommand>(vertex_input_state);
    }

    void CmdUseDepthStencilState(DepthStencilStateBase* depth_stencil_state) {
      Push<UseDepthStencilStateCommand>(depth_stencil_state);
    }

    void CmdSetViewport(f32 x, f32 y, f32 width, f32 height) {
      Push<SetViewportCommand>(x, y, width, height);
    }

    void CmdSetScissor(i32 x, i32 y, u32 width, u32 height) {
      Push<SetScissorCommand>(x, y, width, height);
    }

    void CmdBindVertexBuffer(u32 binding, BufferBase* buffer, u64 buffer_offset) {
      Push<BindVertexBufferCommand>(binding, buffer, buffer_offset);
    }

    void CmdBindResourceSet(u32 index, ResourceSetBase* resource_set) {
      Push<BindResourceSetCommand>(index, resource_set);
    }

    void CmdDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {
      // TODO: validate that enough state is bound for the draw.
      Push<DrawCommand>(vertex_count, instance_count, first_vertex, first_instance);
    }

  private:
    static constexpr size_t k_chunk_size = 65536u;

    // TODO(fleroviux): implement this in a nicer way
    friend class RenderCommandEncoder;

    template<typename T, typename... Args>
    const T& Push(Args&&... args) {
      T* const command = new(AllocateMemory(sizeof(T))) T{std::forward<Args>(args)...};

      if(m_head == nullptr) {
        m_head = command;
        m_tail = command;
      } else {
        m_tail->m_next = command;
        m_tail = command;
      }
      return *command;
    }

    void* AllocateMemory(size_t number_of_bytes) {
      void* address = m_memory_chunks[m_active_chunk].Allocate(number_of_bytes);

      if(address == nullptr) [[unlikely]] {
        if(++m_active_chunk == m_memory_chunks.size()) [[unlikely]] {
          m_memory_chunks.emplace_back(k_chunk_size);
        }
        m_memory_chunks[m_active_chunk].Reset();
        address = m_memory_chunks[m_active_chunk].Allocate(number_of_bytes);
      }

      return address;
    }

    struct State {
      bool has_errors{false};
      bool inside_render_pass{false};
    };

    DeviceBase* m_device;
    std::vector<BumpAllocator> m_memory_chunks{};
    size_t m_active_chunk{};
    CommandBase* m_head{};
    CommandBase* m_tail{};
    State m_state{};
};

}  // namespace mgpu
