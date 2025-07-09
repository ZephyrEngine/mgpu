
#include <mgpu/mgpu.h>
#include <algorithm>

#include "backend/swap_chain.hpp"

extern "C" {

MGPUResult mgpuSwapChainEnumerateTextures(MGPUSwapChain swap_chain, uint32_t* texture_count, MGPUTexture* textures) {
  const auto max_textures = (size_t)*texture_count;

  mgpu::Result<std::span<mgpu::TextureBase* const>> cxx_textures_result = ((mgpu::SwapChainBase*)swap_chain)->EnumerateTextures();
  MGPU_FORWARD_ERROR(cxx_textures_result.Code());

  std::span<mgpu::TextureBase* const> cxx_textures = cxx_textures_result.Unwrap();
  if(cxx_textures.size() > std::numeric_limits<uint32_t>::max()) {
    return MGPU_INTERNAL_ERROR;
  }

  *texture_count = cxx_textures.size();
  if(textures != nullptr) {
    const size_t copy_size = std::min(max_textures, cxx_textures.size());
    std::memcpy(textures, cxx_textures.data(), copy_size * sizeof(MGPUTexture));
    if(copy_size < cxx_textures.size()) {
      return MGPU_INCOMPLETE;
    }
  }

  return MGPU_SUCCESS;
}

MGPUResult mgpuSwapChainAcquireNextTexture(MGPUSwapChain swap_chain, uint32_t* texture_index) {
  const auto cxx_swap_chain = (mgpu::SwapChainBase*)swap_chain;

  if(cxx_swap_chain->WasRetired()) {
    return MGPU_SWAP_CHAIN_RETIRED;
  }
  return cxx_swap_chain->AcquireNextTexture(*texture_index);
}

MGPUResult mgpuSwapChainPresent(MGPUSwapChain swap_chain) {
  return ((mgpu::SwapChainBase*)swap_chain)->Present();
}

}  // extern "C"
