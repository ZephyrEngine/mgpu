
#include <mgpu/mgpu.h>

#include "backend/swap_chain.hpp"

extern "C" {

uint32_t mgpuSwapChainGetNumberOfTextures(MGPUSwapChain swap_chain) {
  return ((mgpu::SwapChainBase*)swap_chain)->GetNumberOfTextures();
}

void mgpuSwapChainDestroy(MGPUSwapChain swap_chain) {
  delete (mgpu::SwapChainBase*)swap_chain;
}

}  // extern "C"
