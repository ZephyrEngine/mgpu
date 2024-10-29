
#include <mgpu/mgpu.h>

#include "backend/queue.hpp"

extern "C" {

MGPUResult mgpuQueueSubmitCommandList(MGPUQueue queue, MGPUCommandList command_list) {
  // TODO(fleroviux): validate that the command list is compatible with the queue
  const auto cxx_queue = (mgpu::QueueBase*)queue;
  const auto cxx_command_list = (const mgpu::CommandList*)command_list;
  if(cxx_command_list->HasErrors()) {
    return MGPU_BAD_COMMAND_LIST;
  }
  return cxx_queue->SubmitCommandList(cxx_command_list);
}

MGPUResult mgpuQueueFlush(MGPUQueue queue) {
  return ((mgpu::QueueBase*)queue)->Flush();
}

} // extern "C"
