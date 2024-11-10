
#include <mgpu/mgpu.h>

#include "backend/vertex_input_state.hpp"

extern "C" {

void mgpuVertexInputStateDestroy(MGPUVertexInputState vertex_input_state) {
  delete (mgpu::VertexInputStateBase*)vertex_input_state;
}

} // extern "C"
