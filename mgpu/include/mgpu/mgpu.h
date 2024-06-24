
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

// ======================================================= //
//   Enumerations                                          //
// ======================================================= //

typedef enum MGPUResult {
  MGPU_SUCCESS = 0,
  MGPU_BAD_ENUM = 1,
  MGPU_INTERNAL_ERROR = 2,

  MGPU_RESERVED = -1
} MGPUResult;

typedef enum MGPUBackend {
  MGPU_BACKEND_OPENGL = 0,
  MGPU_BACKEND_VULKAN = 1
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

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //


// ======================================================= //
//   Object creation / descriptor structures               //
// ======================================================= //

typedef struct MGPUBufferCreateInfo {
  uint64_t size;
  MGPUBufferUsage usage;
} MGPUBufferCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================= //
//   Methods                                               //
// ======================================================= //

const char* mgpuResultCodeToString(MGPUResult result);

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, SDL_Window* sdl_window, MGPURenderDevice* render_device);
void mgpuDestroyRenderDevice(MGPURenderDevice render_device);

MGPUResult mgpuCreateBuffer(MGPURenderDevice render_device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer);
void mgpuDestroyBuffer(MGPURenderDevice render_device, MGPUBuffer buffer);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
