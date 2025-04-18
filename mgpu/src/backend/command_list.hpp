
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

#include "common/bump_allocator.hpp"
#include "common/limits.hpp"
#include "render_command_encoder.hpp"

namespace mgpu {

class TextureViewBase;
class ShaderProgramBase;
class RasterizerStateBase;
class InputAssemblyStateBase;
class ColorBlendStateBase;
class VertexInputStateBase;
class DepthStencilStateBase;
class BufferBase;
class ResourceSetBase;

enum class CommandType {
  BeginRenderPass,
  EndRenderPass,
  UseShaderProgram,
  UseRasterizerState,
  UseInputAssemblyState,
  UseColorBlendState,
  UseVertexInputState,
  UseDepthStencilState,
  SetViewport,
  SetScissor,
  BindVertexBuffer,
  BindResourceSet,
  Draw
};

struct CommandBase {
  explicit CommandBase(CommandType command) : m_command_type{command} {}

  CommandType m_command_type;
  CommandBase* m_next{};
};

struct BeginRenderPassCommand : CommandBase {
  struct ColorAttachment {
    TextureViewBase* texture_view;
    MGPULoadOp load_op;
    MGPUStoreOp store_op;
    MGPUColor clear_color;
  };

  struct DepthStencilAttachment {
    TextureViewBase* texture_view;
    MGPULoadOp depth_load_op;
    MGPUStoreOp depth_store_op;
    MGPULoadOp stencil_load_op;
    MGPUStoreOp stencil_store_op;
    float clear_depth;
    uint32_t clear_stencil;
  };

  explicit BeginRenderPassCommand(const MGPURenderPassBeginInfo& begin_info) : CommandBase{CommandType::BeginRenderPass} {
    for(size_t i = 0; i < begin_info.color_attachment_count; i++) {
      const MGPURenderPassColorAttachment& color_attachment = begin_info.color_attachments[i];
      m_color_attachments.PushBack({
        .texture_view = (TextureViewBase*)color_attachment.texture_view,
        .load_op = color_attachment.load_op,
        .store_op = color_attachment.store_op,
        .clear_color = color_attachment.clear_color
      });
    }

    if(begin_info.depth_stencil_attachment != nullptr) {
      const MGPURenderPassDepthStencilAttachment& depth_stencil_attachment = *begin_info.depth_stencil_attachment;
      m_depth_stencil_attachment = {
        .texture_view = (TextureViewBase*)depth_stencil_attachment.texture_view,
        .depth_load_op = depth_stencil_attachment.depth_load_op,
        .depth_store_op = depth_stencil_attachment.depth_store_op,
        .stencil_load_op = depth_stencil_attachment.stencil_load_op,
        .stencil_store_op = depth_stencil_attachment.stencil_store_op,
        .clear_depth = depth_stencil_attachment.clear_depth,
        .clear_stencil = depth_stencil_attachment.clear_stencil
      };
      m_have_depth_stencil_attachment = true;
    }
  }

  atom::Vector_N<ColorAttachment, limits::max_color_attachments> m_color_attachments{};
  DepthStencilAttachment m_depth_stencil_attachment;
  bool m_have_depth_stencil_attachment{};
};

struct EndRenderPassCommand : CommandBase {
  EndRenderPassCommand() : CommandBase{CommandType::EndRenderPass} {}
};

struct UseShaderProgramCommand : CommandBase {
  explicit UseShaderProgramCommand(ShaderProgramBase* shader_program)
      : CommandBase{CommandType::UseShaderProgram}
      , m_shader_program{shader_program} {
  }

  ShaderProgramBase* m_shader_program;
};

struct UseRasterizerStateCommand : CommandBase {
  explicit UseRasterizerStateCommand(RasterizerStateBase* rasterizer_state)
      : CommandBase{CommandType::UseRasterizerState}
      , m_rasterizer_state{rasterizer_state} {
  }

  RasterizerStateBase* m_rasterizer_state;
};

struct UseInputAssemblyStateCommand : CommandBase {
  explicit UseInputAssemblyStateCommand(InputAssemblyStateBase* input_assembly_state)
      : CommandBase{CommandType::UseInputAssemblyState}
      , m_input_assembly_state{input_assembly_state} {
  }

  InputAssemblyStateBase* m_input_assembly_state;
};

struct UseColorBlendStateCommand : CommandBase {
  explicit UseColorBlendStateCommand(ColorBlendStateBase* color_blend_state)
      : CommandBase{CommandType::UseColorBlendState}
      , m_color_blend_state{color_blend_state} {
  }

  ColorBlendStateBase* m_color_blend_state;
};

struct UseVertexInputStateCommand : CommandBase {
  explicit UseVertexInputStateCommand(VertexInputStateBase* vertex_input_state)
      : CommandBase{CommandType::UseVertexInputState}
      , m_vertex_input_state{vertex_input_state} {
  }

  VertexInputStateBase* m_vertex_input_state;
};

struct UseDepthStencilStateCommand : CommandBase {
  explicit UseDepthStencilStateCommand(DepthStencilStateBase* depth_stencil_state)
      : CommandBase{CommandType::UseDepthStencilState}
      , m_depth_stencil_state{depth_stencil_state} {
  }

  DepthStencilStateBase* m_depth_stencil_state;
};

struct SetViewportCommand : CommandBase {
  SetViewportCommand(f32 x, f32 y, f32 width, f32 height)
      : CommandBase{CommandType::SetViewport}
      , m_viewport_x{x}
      , m_viewport_y{y}
      , m_viewport_width{width}
      , m_viewport_height{height} {
  }

  f32 m_viewport_x;
  f32 m_viewport_y;
  f32 m_viewport_width;
  f32 m_viewport_height;
};

struct SetScissorCommand : CommandBase {
  SetScissorCommand(i32 x, i32 y, u32 width, u32 height)
      : CommandBase{CommandType::SetScissor}
      , m_scissor_x{x}
      , m_scissor_y{y}
      , m_scissor_width{width}
      , m_scissor_height{height} {
  }

  i32 m_scissor_x;
  i32 m_scissor_y;
  u32 m_scissor_width;
  u32 m_scissor_height;
};

struct BindVertexBufferCommand : CommandBase {
  BindVertexBufferCommand(u32 binding, BufferBase* buffer, u64 buffer_offset)
      : CommandBase{CommandType::BindVertexBuffer}
      , m_binding{binding}
      , m_buffer{buffer}
      , m_buffer_offset{buffer_offset} {
  }

  u32 m_binding;
  BufferBase* m_buffer;
  u64 m_buffer_offset;
};

struct BindResourceSetCommand : CommandBase {
  BindResourceSetCommand(u32 index, ResourceSetBase* resource_set)
      : CommandBase{CommandType::BindResourceSet}
      , m_index{index}
      , m_resource_set{resource_set} {
  }

  u32 m_index;
  ResourceSetBase* m_resource_set;
};

struct DrawCommand : CommandBase {
  DrawCommand(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
      : CommandBase{CommandType::Draw}
      , m_vertex_count{vertex_count}
      , m_instance_count{instance_count}
      , m_first_vertex{first_vertex}
      , m_first_instance{first_instance} {
  }

  u32 m_vertex_count;
  u32 m_instance_count;
  u32 m_first_vertex;
  u32 m_first_instance;
};

class CommandList : atom::NonCopyable, atom::NonMoveable {
  public:
    CommandList() {
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
      // TODO(fleroviux): just allocate the encoder inside the command itself? That way we safe one alloc.
      const auto render_command_encoder = new(AllocateMemory(sizeof(RenderCommandEncoder))) RenderCommandEncoder{this};

      if(m_state.inside_render_pass) {
        m_state.has_errors = true;
      }
      m_state.inside_render_pass = true;
      Push<BeginRenderPassCommand>(begin_info);
      return render_command_encoder;
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
    void Push(Args&&... args) {
      CommandBase* command = new(AllocateMemory(sizeof(T))) T{std::forward<Args>(args)...};

      if(m_head == nullptr) {
        m_head = command;
        m_tail = command;
      } else {
        m_tail->m_next = command;
        m_tail = command;
      }
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

    std::vector<BumpAllocator> m_memory_chunks{};
    size_t m_active_chunk{};
    CommandBase* m_head{};
    CommandBase* m_tail{};
    State m_state;
};

}  // namespace mgpu
