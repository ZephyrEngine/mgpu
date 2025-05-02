
#pragma once

#include <atom/integer.hpp>
#include <atom/float.hpp>

namespace mgpu {

class CommandList;
//class TextureViewBase;
class ShaderProgramBase;
class RasterizerStateBase;
class InputAssemblyStateBase;
class ColorBlendStateBase;
class VertexInputStateBase;
class DepthStencilStateBase;
class BufferBase;
class ResourceSetBase;

class RenderCommandEncoder {
  public:
    explicit RenderCommandEncoder(CommandList* command_list) : m_command_list{command_list} {
    }

    void CmdUseShaderProgram(ShaderProgramBase* shader_program);
    void CmdUseRasterizerState(RasterizerStateBase* rasterizer_state);
    void CmdUseInputAssemblyState(InputAssemblyStateBase* input_assembly_state);
    void CmdUseColorBlendState(ColorBlendStateBase* color_blend_state);
    void CmdUseVertexInputState(VertexInputStateBase* vertex_input_state);
    void CmdUseDepthStencilState(DepthStencilStateBase* depth_stencil_state);
    void CmdSetViewport(f32 x, f32 y, f32 width, f32 height);
    void CmdSetScissor(i32 x, i32 y, u32 width, u32 height);
    void CmdBindVertexBuffer(u32 binding, BufferBase* buffer, u64 buffer_offset);
    void CmdBindIndexBuffer(BufferBase* buffer, u64 buffer_offset, MGPUIndexFormat index_format);
    void CmdBindResourceSet(u32 index, ResourceSetBase* resource_set);
    void CmdDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
    void CmdDrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance);
    void Close();

  private:
    CommandList* m_command_list;
};

} // namespace mgpu