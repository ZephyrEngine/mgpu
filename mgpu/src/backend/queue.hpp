
#pragma once

#include <mgpu/mgpu.h>

#include <atom/non_copyable.hpp>
#include <atom/non_moveable.hpp>

#include "backend/command_list/command_list.hpp"

namespace mgpu {

class BufferBase;
class TextureBase;

class QueueBase : atom::NonCopyable, atom::NonMoveable {
  public:
    virtual ~QueueBase() = default;

    virtual MGPUResult SubmitCommandList(const CommandList* command_list) = 0;
    virtual MGPUResult BufferUpload(const BufferBase* buffer, std::span<const u8> data, u64 offset) = 0;
    virtual MGPUResult TextureUpload(const TextureBase* texture, const MGPUTextureUploadRegion& region, const void* data) = 0;
    virtual MGPUResult Flush() = 0;
};

} // namespace mgpu
