
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"

extern "C" {

MGPUResult mgpuCommandListClear(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->Clear();
  return MGPU_SUCCESS;
}

void mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, const MGPURenderPassBeginInfo* begin_info) {
  // TODO(fleroviux): validate input data (see current render target validation code)
  ((mgpu::CommandList*)command_list)->CmdBeginRenderPass(*begin_info);
}

void mgpuCommandListCmdEndRenderPass(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->CmdEndRenderPass();
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

}  // extern "C"
