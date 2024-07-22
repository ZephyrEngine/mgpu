
#include <mgpu/mgpu.h>
#include <atom/panic.hpp>

extern "C" {

const char* mgpuResultCodeToString(MGPUResult result) {
  #define REGISTER(result_code) case result_code: return "" # result_code; break;

  switch(result) {
    REGISTER(MGPU_SUCCESS)
    REGISTER(MGPU_BAD_ENUM)
    REGISTER(MGPU_OUT_OF_MEMORY)
    REGISTER(MGPU_BAD_DIMENSIONS)
    REGISTER(MGPU_INTERNAL_ERROR)
    REGISTER(MGPU_BUFFER_NOT_HOST_VISIBLE)
    REGISTER(MGPU_BUFFER_NOT_MAPPED)
    default: ATOM_PANIC("internal error (missing result code to string translation)")
  }

  #undef REGISTER
}

}  // extern "C"
