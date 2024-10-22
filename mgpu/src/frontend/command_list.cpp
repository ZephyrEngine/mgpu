
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"

extern "C" {

MGPUResult mgpuCommandListClear(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->Clear();
  return MGPU_SUCCESS;
}

void mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, const MGPURenderPassBeginInfo* begin_info) {
  /**
   * TODO(fleroviux): validate begin_info.
   *
   * Some ideas for validation:
   *  - Must have at least one attachment
   *  - Must not have more than the allowed number of color attachments
   *  - All attachments must have the same dimensions
   *  - All attachments must have RENDER_ATTACHMENT usage
   *  - All attachments must be of type MGPU_TEXTURE_VIEW_TYPE_2D (or MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY, if we support layers in the future)
   *  - All color attachments must have a format with color aspect
   *  - The depth/stencil attachment must have a format with a depth or stencil aspect
   */

  ((mgpu::CommandList*)command_list)->CmdBeginRenderPass(*begin_info);
}

void mgpuCommandListCmdEndRenderPass(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->CmdEndRenderPass();
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

}  // extern "C"
