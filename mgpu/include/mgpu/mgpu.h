
#ifndef MGPU_H
#define MGPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// ======================================================= //
//   Object type declarations                              //
// ======================================================= //

typedef struct MGPURenderDeviceImpl* MGPURenderDevice;
typedef struct MGPUBufferImpl* MGPUBuffer;
typedef void* MGPUFence;

// ======================================================= //
//   Enumerations                                          //
// ======================================================= //

typedef enum MGPUResult {
  MGPU_SUCCESS = 0,
  MGPU_BAD_ENUM = 1,
  MGPU_INTERNAL_ERROR = 2,
  MGPU_OUT_OF_MEMORY = 3,
  MGPU_BAD_DIMENSIONS = 4,
  MGPU_BUFFER_NOT_HOST_VISIBLE = 5,
  MGPU_BUFFER_NOT_MAPPED = 6,

  MGPU_RESERVED = -1
} MGPUResult;

typedef enum MGPUBackend {
  MGPU_BACKEND_VULKAN = 0
} MGPUBackend;

typedef enum MGPUBufferUsage {
  MGPU_BUFFER_USAGE_COPY_SRC = 0x00000001,
  MGPU_BUFFER_USAGE_COPY_DST = 0x00000002,
  MGPU_BUFFER_USAGE_UNIFORM_BUFFER = 0x00000010,
  MGPU_BUFFER_USAGE_STORAGE_BUFFER = 0x00000020,
  MGPU_BUFFER_USAGE_INDEX_BUFFER = 0x00000040,
  MGPU_BUFFER_USAGE_VERTEX_BUFFER = 0x00000080,
  MGPU_BUFFER_USAGE_INDIRECT_BUFFER = 0x00000100
} MGPUBufferUsage;

typedef enum MGPUBufferFlags {
  MGPU_BUFFER_FLAGS_HOST_VISIBLE = 0x00000001,
  MGPU_BUFFER_FLAGS_HOST_RANDOM_ACCESS = 0x00000002
} MGPUBufferFlags;

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //


// ======================================================= //
//   Object creation / descriptor structures               //
// ======================================================= //

typedef struct MGPUBufferCreateInfo {
  uint64_t size;
  MGPUBufferUsage usage; // TODO(fleroviux): fix broken OR-ing behavior
  MGPUBufferFlags flags; // TODO(fleroviux): fix broken OR-ing behavior
} MGPUBufferCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================= //
//   Methods                                               //
// ======================================================= //

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, SDL_Window* sdl_window, MGPURenderDevice* render_device);
void mgpuDestroyRenderDevice(MGPURenderDevice render_device);

MGPUResult mgpuCreateBuffer(MGPURenderDevice render_device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer);
MGPUResult mgpuMapBuffer(MGPURenderDevice render_device, MGPUBuffer buffer, void** address);
MGPUResult mgpuUnmapBuffer(MGPURenderDevice render_device, MGPUBuffer buffer);
MGPUResult mgpuFlushBuffer(MGPURenderDevice render_device, MGPUBuffer buffer, uint64_t offset, uint64_t size);
void mgpuDestroyBuffer(MGPURenderDevice render_device, MGPUBuffer buffer);

MGPUFence mgpuFenceSync(MGPURenderDevice render_device);
MGPUResult mgpuWaitFence(MGPURenderDevice render_device, MGPUFence fence);

const char* mgpuResultCodeToString(MGPUResult result);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
