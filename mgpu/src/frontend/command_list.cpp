
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

void mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, MGPURenderTarget render_target) {
  ((mgpu::CommandList*)command_list)->CmdBeginRenderPass((mgpu::RenderTargetBase*)render_target);
}

void mgpuCommandListCmdEndRenderPass(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->CmdEndRenderPass();
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

}  // extern "C"
