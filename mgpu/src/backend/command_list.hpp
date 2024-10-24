
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/panic.hpp>
#include <atom/vector_n.hpp>
#include <utility>
#include <vector>

#include "backend/texture.hpp"
#include "common/bump_allocator.hpp"
#include "common/limits.hpp"

namespace mgpu {

enum class CommandType {
  BeginRenderPass,
  EndRenderPass
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

    void CmdBeginRenderPass(const MGPURenderPassBeginInfo& begin_info) {
      if(m_state.inside_render_pass) {
        m_state.has_errors = true;
      }
      m_state.inside_render_pass = true;

      Push<BeginRenderPassCommand>(begin_info);
    }

    void CmdEndRenderPass() {
      if(!m_state.inside_render_pass) {
        m_state.has_errors = true;
      }
      m_state.inside_render_pass = false;

      Push<EndRenderPassCommand>();
    }

  private:
    static constexpr size_t k_chunk_size = 65536;

    template<typename T, typename... Args>
    void Push(Args&&... args) {
      CommandBase* command = new(AllocateCommandMemory(sizeof(T))) T{std::forward<Args>(args)...};

      if(m_head == nullptr) {
        m_head = command;
        m_tail = command;
      } else {
        m_tail->m_next = command;
        m_tail = command;
      }
    }

    void* AllocateCommandMemory(size_t number_of_bytes) {
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
