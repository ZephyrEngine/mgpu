
#include <mgpu/mgpu.h>

#include "backend/queue.hpp"
#include "validation/buffer.hpp"

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

MGPUResult mgpuQueueBufferUpload(MGPUQueue queue, MGPUBuffer buffer, uint64_t offset, uint64_t size, const void* data) {
  const auto cxx_queue = (mgpu::QueueBase*)queue;
  const auto cxx_buffer = (mgpu::BufferBase*)buffer;

  MGPU_FORWARD_ERROR(validate_buffer_range(cxx_buffer, offset, size));
  MGPU_FORWARD_ERROR(validate_buffer_has_usage_bits(cxx_buffer, MGPU_BUFFER_USAGE_COPY_DST));
  if(size == 0u) {
    return MGPU_SUCCESS;
  }
  return cxx_queue->BufferUpload(cxx_buffer, {(const u8*)data, size}, offset);
}

MGPUResult mgpuQueueFlush(MGPUQueue queue) {
  return ((mgpu::QueueBase*)queue)->Flush();
}

} // extern "C"
