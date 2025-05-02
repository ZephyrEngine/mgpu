
#pragma once

#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/float.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/vector_n.hpp>

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
  BindIndexBuffer,
  BindResourceSet,
  Draw,
  DrawIndexed
};

struct CommandBase : atom::NonCopyable, atom::NonMoveable {
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

  BeginRenderPassCommand(CommandList* command_list, const MGPURenderPassBeginInfo& begin_info)
      : CommandBase{CommandType::BeginRenderPass}
      , m_render_command_encoder{command_list} {
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

  mutable RenderCommandEncoder m_render_command_encoder;
  atom::Vector_N<ColorAttachment, limits::max_color_attachments> m_color_attachments{};
  DepthStencilAttachment m_depth_stencil_attachment{};
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

struct BindIndexBufferCommand : CommandBase {
  BindIndexBufferCommand(BufferBase* buffer, u64 buffer_offset, MGPUIndexFormat index_format)
      : CommandBase{CommandType::BindIndexBuffer}
      , m_buffer{buffer}
      , m_buffer_offset{buffer_offset}
      , m_index_format{index_format} {
  }

  BufferBase* m_buffer;
  u64 m_buffer_offset;
  MGPUIndexFormat m_index_format;
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

struct DrawIndexedCommand: CommandBase {
  DrawIndexedCommand(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
      : CommandBase{CommandType::DrawIndexed}
      , m_index_count{index_count}
      , m_instance_count{instance_count}
      , m_first_index{first_index}
      , m_vertex_offset{vertex_offset}
      , m_first_instance{first_instance} {
  }

  u32 m_index_count;
  u32 m_instance_count;
  u32 m_first_index;
  i32 m_vertex_offset;
  u32 m_first_instance;
};

} // namespace mgpu
