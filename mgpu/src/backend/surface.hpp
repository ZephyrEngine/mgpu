
#pragma once

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "swap_chain.hpp"

namespace mgpu {

class SurfaceBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~SurfaceBase() = default;

    [[nodiscard]] SwapChainBase* GetAssociatedSwapChain() {
      return m_associated_swap_chain;
    }

    void SetAssociatedSwapChain(SwapChainBase* swap_chain) {
      if(m_associated_swap_chain) {
        m_associated_swap_chain->Retire();
      }
      m_associated_swap_chain = swap_chain;
    }

  private:
    SwapChainBase* m_associated_swap_chain{};
};

}  // namespace mgpu
