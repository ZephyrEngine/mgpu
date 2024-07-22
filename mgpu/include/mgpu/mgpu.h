
#ifndef MGPU_H
#define MGPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define MGPU_NULL_HANDLE ((void*)0u)
#define MGPU_WHOLE_SIZE (~0ull)
#define MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE 256u

// ======================================================= //
//   Object type declarations                              //
// ======================================================= //

typedef struct MGPUInstanceImpl* MGPUInstance;
typedef struct MGPUPhysicalDeviceImpl* MGPUPhysicalDevice;
typedef struct MGPUDeviceImpl* MGPUDevice;
typedef struct MGPUBufferImpl* MGPUBuffer;

// ======================================================= //
//   Enumerations                                          //
// ======================================================= //

typedef uint32_t MGPUFlags;

typedef enum MGPUResult {
  MGPU_SUCCESS = 0,
  MGPU_BAD_ENUM = 1,
  MGPU_OUT_OF_MEMORY = 2,
  MGPU_INTERNAL_ERROR = 3,
  MGPU_BAD_DIMENSIONS = 4,
  MGPU_BUFFER_NOT_HOST_VISIBLE = 5,
  MGPU_BUFFER_NOT_MAPPED = 6,

  MGPU_RESERVED = -1
} MGPUResult;

typedef enum MGPUBackendType {
  MGPU_BACKEND_TYPE_VULKAN = 0
} MGPUBackend;

typedef enum MGPUPhysicalDeviceType {
  MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 0,
  MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
  MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 2
} MGPUPhysicalDeviceType;

typedef enum MGPUBufferUsageBits {
  MGPU_BUFFER_USAGE_COPY_SRC = 0x00000001,
  MGPU_BUFFER_USAGE_COPY_DST = 0x00000002,
  MGPU_BUFFER_USAGE_UNIFORM_BUFFER = 0x00000010,
  MGPU_BUFFER_USAGE_STORAGE_BUFFER = 0x00000020,
  MGPU_BUFFER_USAGE_INDEX_BUFFER = 0x00000040,
  MGPU_BUFFER_USAGE_VERTEX_BUFFER = 0x00000080,
  MGPU_BUFFER_USAGE_INDIRECT_BUFFER = 0x00000100
} MGPUBufferUsageBits;

typedef MGPUFlags MGPUBufferUsage;

typedef enum MGPUBufferFlagsBits {
  MGPU_BUFFER_FLAGS_HOST_VISIBLE = 0x00000001,
  MGPU_BUFFER_FLAGS_HOST_RANDOM_ACCESS = 0x00000002
} MGPUBufferFlagsBits;

typedef MGPUFlags MGPUBufferFlags;

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //


// ======================================================= //
//   Object creation / descriptor structures               //
// ======================================================= //

typedef struct MGPUBufferCreateInfo {
  uint64_t size;
  MGPUBufferUsage usage;
  MGPUBufferFlags flags;
} MGPUBufferCreateInfo;

// ======================================================= //
//   Other structure definitions                           //
// ======================================================= //

struct MGPUPhysicalDeviceLimits {
};

struct MGPUPhysicalDeviceInfo {
  char device_name[MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE];
  MGPUPhysicalDeviceType device_type;
  MGPUPhysicalDeviceLimits limits;
};

#ifdef __cplusplus
extern "C" {
#endif

MGPUResult mgpuCreateInstance(MGPUBackendType backend_type, MGPUInstance* instance);

const char* mgpuResultCodeToString(MGPUResult result);

// ======================================================= //
//   Methods                                               //
// ======================================================= //

// MGPUInstance methods
MGPUResult mgpuInstanceEnumeratePhysicalDevices(MGPUInstance instance, uint32_t* physical_device_count, MGPUPhysicalDevice* physical_devices);
void mgpuInstanceDestroy(MGPUInstance instance);

// MGPUPhysicalDevice methods
MGPUResult mgpuPhysicalDeviceGetInfo(MGPUPhysicalDevice physical_device, MGPUPhysicalDeviceInfo* physical_device_info);
MGPUResult mgpuPhysicalDeviceCreateDevice(MGPUPhysicalDevice physical_device, MGPUDevice* device);

// MGPUDevice methods
MGPUResult mgpuDeviceCreateBuffer(MGPUDevice device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer);
void mgpuDeviceDestroy(MGPUDevice device);

// MGPUBuffer methods
MGPUResult mgpuBufferMap(MGPUBuffer buffer, void** address);
MGPUResult mgpuBufferUnmap(MGPUBuffer buffer);
MGPUResult mgpuBufferFlushRange(MGPUBuffer buffer, uint64_t offset, uint64_t size);
void mgpuBufferDestroy(MGPUBuffer buffer);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
