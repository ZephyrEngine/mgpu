
#pragma once

#include <atom/integer.hpp>
#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>
#include <atom/panic.hpp>
#include <utility>
#include <vector>

#include "backend/texture.hpp"
#include "common/bump_allocator.hpp"

namespace mgpu {

enum class CommandType {
  NoOp,
  Test
};

struct CommandBase {
  explicit CommandBase(CommandType command) : m_command_type{command} {}

  CommandType m_command_type;
  CommandBase* m_next{};
};

struct TestCommand : CommandBase {
  explicit TestCommand(TextureBase* texture) : CommandBase{CommandType::Test}, m_texture{texture} {}

  TextureBase* m_texture;
};

class CommandList : atom::NonCopyable, atom::NonMoveable {
  public:
    CommandList() {
      m_memory_chunks.emplace_back(k_chunk_size);
    }

    [[nodiscard]] const CommandBase* GetListHead() const {
      return m_head;
    }

    void Clear() {
      m_memory_chunks[0].Reset();
      m_active_chunk = 0u;
      m_head = nullptr;
      m_tail = nullptr;
    }

    void CmdTest(TextureBase* texture) {
      Push<TestCommand>(texture);
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

    std::vector<BumpAllocator> m_memory_chunks{};
    size_t m_active_chunk{};
    CommandBase* m_head{};
    CommandBase* m_tail{};
};

}  // namespace mgpu
