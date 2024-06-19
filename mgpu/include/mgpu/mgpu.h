
#ifndef MGPU_H
#define MGPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// ======================================================= //
//   Object type declarations                              //
// ======================================================= //

typedef struct MGPURenderDeviceImpl* MGPURenderDevice;

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

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //


// ======================================================= //
//   Object creation / descriptor structures               //
// ======================================================= //

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================= //
//   Methods                                               //
// ======================================================= //

const char* mgpuResultCodeToString(MGPUResult result);

MGPUResult mgpuCreateRenderDevice(MGPUBackend backend, SDL_Window* sdl_window, MGPURenderDevice* render_device);
void mgpuDestroyRenderDevice(MGPURenderDevice render_device);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
