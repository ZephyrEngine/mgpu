
#include <mgpu/mgpu.h>

#include "backend/swap_chain.hpp"

extern "C" {

void mgpuSwapChainDestroy(MGPUSwapChain swap_chain) {
  delete (mgpu::SwapChainBase*)swap_chain;
}

}  // extern "C"
