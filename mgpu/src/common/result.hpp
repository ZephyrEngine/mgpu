
#pragma once

#include <mgpu/mgpu.h>
#include <atom/result.hpp>

namespace mgpu {

  #define MGPU_FORWARD_ERROR(result_code) do { \
    if(result_code != MGPU_SUCCESS) { \
      return result_code;\
    } \
  } while(0);

  template<typename TValue>
  using Result = atom::Result<MGPUResult, TValue>;

}  // namespace mgpu
