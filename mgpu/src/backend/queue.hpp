
#pragma once

#include <mgpu/mgpu.h>

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "backend/command_list.hpp"

namespace mgpu {

class QueueBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~QueueBase() = default;

    virtual MGPUResult SubmitCommandList(const CommandList* command_list) = 0;
    virtual MGPUResult Flush() = 0;
};

} // namespace mgpu
