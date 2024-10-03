
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/panic.hpp>
#include <utility>
#include <vector>

#include "backend/render_target.hpp"
#include "backend/texture.hpp"
#include "common/bump_allocator.hpp"

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
  explicit BeginRenderPassCommand(RenderTargetBase* render_target) : CommandBase{CommandType::BeginRenderPass}, m_render_target{render_target} {}

  RenderTargetBase* m_render_target;
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

    void CmdBeginRenderPass(RenderTargetBase* render_target) {
      if(m_state.inside_render_pass) {
        m_state.has_errors = true;
      }
      m_state.inside_render_pass = true;

      Push<BeginRenderPassCommand>(render_target);
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
