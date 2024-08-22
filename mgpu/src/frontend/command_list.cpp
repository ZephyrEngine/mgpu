
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"

extern "C" {

MGPUResult mgpuCommandListClear(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->Clear();
  return MGPU_SUCCESS;
}

void mgpuCommandListCmdTest(MGPUCommandList command_list, MGPUTexture texture) {
  ((mgpu::CommandList*)command_list)->CmdTest((mgpu::TextureBase*)texture);
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

}  // extern "C"
