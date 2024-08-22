
#pragma once

#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <cstdlib>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace mgpu {

class BumpAllocator {
  public:
    explicit BumpAllocator(size_t capacity) {
#ifdef WIN32
      m_base_address = (u8*)VirtualAlloc(nullptr, capacity, MEM_COMMIT, PAGE_READWRITE);
#else
      m_data = (u8*)std::malloc(capacity);
#endif

      if(m_base_address == nullptr) {
        ATOM_PANIC("out of memory");
      }
      m_maximum_address = m_base_address + capacity;
      Reset();
    }

   ~BumpAllocator() {
      const size_t capacity = m_maximum_address - m_base_address;

#ifdef WIN32
      VirtualFree(m_base_address, capacity, MEM_RELEASE);
#else
      std::free(m_data);
#endif
    }

    void Reset() {
      m_current_address = m_base_address;
    }

    void* Allocate(size_t number_of_bytes) {
      u8* address = m_current_address;
      m_current_address += number_of_bytes;
      if(m_current_address <= m_maximum_address) {
        return address;
      }
      return nullptr;
    }

  private:
    u8* m_base_address{};
    u8* m_current_address{};
    u8* m_maximum_address{};
};

}  // namespace mgpu
