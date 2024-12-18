
#include <mgpu/mgpu.h>

#include "backend/input_assembly_state.hpp"

extern "C" {

void mgpuInputAssemblyStateDestroy(MGPUInputAssemblyState input_assembly_state) {
  delete (mgpu::InputAssemblyStateBase*)input_assembly_state;
}

} // extern "C"
