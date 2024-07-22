
#pragma once

#include <mgpu/mgpu.h>
#include <atom/result.hpp>

namespace mgpu {

#define MGPU_FORWARD_ERROR(expression) do { \
  const MGPUResult mgpu_result = (expression); \
  if(mgpu_result != MGPU_SUCCESS) { \
    return mgpu_result;\
  } \
} while(0)

template<typename TValue>
using Result = atom::Result<MGPUResult, TValue>;

}  // namespace mgpu
