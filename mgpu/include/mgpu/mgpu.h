
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
typedef struct MGPUTextureImpl* MGPUTexture;
typedef struct MGPUTextureViewImpl* MGPUTextureView;

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
  MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE = 8,
  MGPU_INCOMPATIBLE_TEXTURE_FORMAT = 9,
  MGPU_INCOMPATIBLE_TEXTURE_ASPECT = 10
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

typedef enum MGPUTextureFormat {
  MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB = 0
} MGPUTextureFormat;

typedef enum MGPUTextureUsageBits {
  MGPU_TEXTURE_USAGE_COPY_SRC = 0x00000001,
  MGPU_TEXTURE_USAGE_COPY_DST = 0x00000002,
  MGPU_TEXTURE_USAGE_SAMPLED = 0x00000004,
  MGPU_TEXTURE_USAGE_STORAGE = 0x00000008,
  MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT = 0x00000010
} MGPUTextureUsageBits;

typedef MGPUFlags MGPUTextureUsage;

typedef enum MGPUTextureType {
  MGPU_TEXTURE_TYPE_1D = 0,
  MGPU_TEXTURE_TYPE_2D = 1,
  MGPU_TEXTURE_TYPE_3D = 2
} MGPUTextureType;

typedef enum MGPUTextureAspectBits {
  MGPU_TEXTURE_ASPECT_COLOR = 0x00000001,
  MGPU_TEXTURE_ASPECT_DEPTH = 0x00000002,
  MGPU_TEXTURE_ASPECT_STENCIL = 0x00000004
} MGPUTextureAspectBits;

typedef MGPUFlags MGPUTextureAspect;

typedef enum MGPUTextureViewType {
  MGPU_TEXTURE_VIEW_TYPE_1D = 0,
  MGPU_TEXTURE_VIEW_TYPE_2D = 1,
  MGPU_TEXTURE_VIEW_TYPE_3D = 2,
  MGPU_TEXTURE_VIEW_TYPE_CUBE = 3,
  MGPU_TEXTURE_VIEW_TYPE_1D_ARRAY = 4,
  MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY = 5,
  MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY = 6
} MGPUTextureViewType;

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //

typedef struct MGPUExtent3D {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
} MGPUExtent3D;

// ======================================================= //
//   Object creation / descriptor structures               //
// ======================================================= //

typedef struct MGPUBufferCreateInfo {
  uint64_t size;
  MGPUBufferUsage usage;
  MGPUBufferFlags flags;
} MGPUBufferCreateInfo;

typedef struct MGPUTextureCreateInfo {
  MGPUTextureFormat format;
  MGPUTextureType type;
  MGPUExtent3D extent;
  uint32_t mip_count;
  uint32_t array_layer_count;
  MGPUTextureUsage usage;
} MGPUTextureCreateInfo;

typedef struct MGPUTextureViewCreateInfo {
  MGPUTextureViewType type;
  MGPUTextureFormat format;
  MGPUTextureAspect aspect;
  uint32_t base_mip;
  uint32_t mip_count;
  uint32_t base_array_layer;
  uint32_t array_layer_count;
} MGPUTextureViewCreateInfo;

// ======================================================= //
//   Other structure definitions                           //
// ======================================================= //

struct MGPUPhysicalDeviceLimits {
  uint32_t max_texture_dimension_1d;
  uint32_t max_texture_dimension_2d;
  uint32_t max_texture_dimension_3d;
  uint32_t max_texture_array_layers;
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
MGPUResult mgpuDeviceCreateTexture(MGPUDevice device, const MGPUTextureCreateInfo* create_info, MGPUTexture* texture);
void mgpuDeviceDestroy(MGPUDevice device);

// MGPUBuffer methods
MGPUResult mgpuBufferMap(MGPUBuffer buffer, void** address);
MGPUResult mgpuBufferUnmap(MGPUBuffer buffer);
MGPUResult mgpuBufferFlushRange(MGPUBuffer buffer, uint64_t offset, uint64_t size);
void mgpuBufferDestroy(MGPUBuffer buffer);

// MGPUTexture methods
MGPUResult mgpuTextureCreateView(MGPUTexture texture, const MGPUTextureViewCreateInfo* create_info, MGPUTextureView* texture_view);
void mgpuTextureDestroy(MGPUTexture texture);

// MGPUTextureView methods
void mgpuTextureViewDestroy(MGPUTextureView texture_view);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
