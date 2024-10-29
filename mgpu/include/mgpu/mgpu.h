
#ifndef MGPU_H
#define MGPU_H

#include <stdint.h>
#include <stdbool.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

#ifdef __APPLE__
  #ifdef __OBJC__
    @class CAMetalLayer;
  #else
    typedef void CAMetalLayer;
  #endif
#endif

#define MGPU_NULL_HANDLE ((void*)0u)
#define MGPU_WHOLE_SIZE (~0ull)
#define MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE 256u

// ======================================================= //
//   Object type declarations                              //
// ======================================================= //

typedef struct MGPUInstanceImpl* MGPUInstance;
typedef struct MGPUPhysicalDeviceImpl* MGPUPhysicalDevice;
typedef struct MGPUDeviceImpl* MGPUDevice;
typedef struct MGPUQueueImpl* MGPUQueue;
typedef struct MGPUBufferImpl* MGPUBuffer;
typedef struct MGPUTextureImpl* MGPUTexture;
typedef struct MGPUTextureViewImpl* MGPUTextureView;
typedef struct MGPUCommandListImpl* MGPUCommandList;
typedef struct MGPUSurfaceImpl* MGPUSurface;
typedef struct MGPUSwapChainImpl* MGPUSwapChain;

// ======================================================= //
//   Enumerations                                          //
// ======================================================= //

typedef uint32_t MGPUFlags;

typedef enum MGPUResult {
  MGPU_SUCCESS = 0,
  MGPU_BAD_ENUM = 1,
  MGPU_OUT_OF_MEMORY = 2,
  MGPU_INTERNAL_ERROR = 3,
  MGPU_INCOMPLETE = 4,
  MGPU_BAD_DIMENSIONS = 5,
  MGPU_BUFFER_NOT_HOST_VISIBLE = 6,
  MGPU_BUFFER_NOT_MAPPED = 7,
  MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE = 8,
  MGPU_INCOMPATIBLE_TEXTURE_FORMAT = 9,
  MGPU_INCOMPATIBLE_TEXTURE_USAGE  = 10,
  MGPU_INCOMPATIBLE_TEXTURE_ASPECT = 11,
  MGPU_NOT_READY = 12,
  MGPU_BAD_COMMAND_LIST = 13
} MGPUResult;

typedef enum MGPUBackendType {
  MGPU_BACKEND_TYPE_VULKAN = 0
} MGPUBackend;

typedef enum MGPUQueueType {
  MGPU_QUEUE_TYPE_GRAPHICS_COMPUTE = 0,
  MGPU_QUEUE_TYPE_ASYNC_COMPUTE = 1
} MGPUQueueType;

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

typedef enum MGPULoadOp {
  MGPU_LOAD_OP_LOAD = 0,
  MGPU_LOAD_OP_CLEAR = 1,
  MGPU_LOAD_OP_DONT_CARE = 2
} MGPULoadOp;

typedef enum MGPUStoreOp {
  MGPU_STORE_OP_STORE = 0,
  MGPU_STORE_OP_DONT_CARE = 1
} MGPUStoreOp;

typedef enum MGPUColorSpace {
  MGPU_COLOR_SPACE_SRGB_NONLINEAR = 0
} MGPUColorSpace;

typedef enum MGPUPresentMode {
  MGPU_PRESENT_MODE_IMMEDIATE = 0,
  MGPU_PRESENT_MODE_MAILBOX = 1,
  MGPU_PRESENT_MODE_FIFO = 2,
  MGPU_PRESENT_MODE_FIFO_RELAXED = 3
} MGPUPresentMode;

// ======================================================= //
//   Common structure definitions                          //
// ======================================================= //

typedef struct MGPUExtent2D {
  uint32_t width;
  uint32_t height;
} MGPUExtent2D;

typedef struct MGPUExtent3D {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
} MGPUExtent3D;

typedef struct MGPUColor {
  double r;
  double g;
  double b;
  double a;
} MGPUColor;

typedef struct MGPUSurfaceCapabilities {
  // TODO(fleroviux): might want to expose composite alpha, pre-transform and array layer count settings?
  uint32_t min_texture_count;
  uint32_t max_texture_count;
  MGPUExtent2D current_extent;
  MGPUExtent2D min_texture_extent;
  MGPUExtent2D max_texture_extent;
  MGPUTextureUsage supported_usage;
} MGPUSurfaceCapabilities;

typedef struct MGPUSurfaceFormat {
  MGPUTextureFormat format;
  MGPUColorSpace  color_space;
} MGPUSurfaceFormat;

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

typedef struct MGPUSurfaceCreateInfo {
#ifdef WIN32
  struct {
    HINSTANCE hinstance;
    HWND hwnd;
  } win32;
#endif
#ifdef __APPLE__
  struct {
    const CAMetalLayer* metal_layer;
  } metal;
#endif
} MGPUSurfaceCreateInfo;

typedef struct MGPUSwapChainCreateInfo {
  // TODO(fleroviux): might want to expose composite alpha, pre-transform and array layer count settings?
  MGPUSurface surface;
  MGPUTextureFormat format;
  MGPUColorSpace color_space;
  MGPUPresentMode present_mode;
  MGPUTextureUsage usage;
  MGPUExtent2D extent;
  uint32_t min_texture_count;
  MGPUSwapChain old_swap_chain;
} MGPUSwapChainCreateInfo;

// ======================================================= //
//   Other structure definitions                           //
// ======================================================= //

typedef struct MGPUPhysicalDeviceLimits {
  uint32_t max_texture_dimension_1d;
  uint32_t max_texture_dimension_2d;
  uint32_t max_texture_dimension_3d;
  uint32_t max_texture_array_layers;
  uint32_t max_color_attachments;
  uint32_t max_attachment_dimension;
} MGPUPhysicalDeviceLimits;

typedef struct MGPUPhysicalDeviceInfo {
  char device_name[MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE];
  MGPUPhysicalDeviceType device_type;
  MGPUPhysicalDeviceLimits limits;
} MGPUPhysicalDeviceInfo;

typedef struct MGPURenderPassColorAttachment {
  MGPUTextureView texture_view;
  MGPULoadOp load_op;
  MGPUStoreOp store_op;
  MGPUColor clear_color;
} MGPURenderPassColorAttachment;

typedef struct MGPURenderPassDepthStencilAttachment {
  MGPUTextureView texture_view;
  MGPULoadOp depth_load_op;
  MGPUStoreOp depth_store_op;
  MGPULoadOp stencil_load_op;
  MGPUStoreOp stencil_store_op;
  float clear_depth;
  uint32_t clear_stencil;
} MGPURenderPassDepthStencilAttachment;

typedef struct MGPURenderPassBeginInfo {
  uint32_t color_attachment_count;
  const MGPURenderPassColorAttachment* color_attachments;
  const MGPURenderPassDepthStencilAttachment* depth_stencil_attachment;
} MGPURenderPassBeginInfo;

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
MGPUResult mgpuInstanceCreateSurface(MGPUInstance instance, const MGPUSurfaceCreateInfo* create_info, MGPUSurface* surface);
void mgpuInstanceDestroy(MGPUInstance instance);

// MGPUPhysicalDevice methods
MGPUResult mgpuPhysicalDeviceGetInfo(MGPUPhysicalDevice physical_device, MGPUPhysicalDeviceInfo* physical_device_info);
MGPUResult mgpuPhysicalDeviceGetSurfaceCapabilities(MGPUPhysicalDevice physical_device, MGPUSurface surface, MGPUSurfaceCapabilities* surface_capabilities);
MGPUResult mgpuPhysicalDeviceEnumerateSurfaceFormats(MGPUPhysicalDevice physical_device, MGPUSurface surface, uint32_t* surface_format_count, MGPUSurfaceFormat* surface_formats);
MGPUResult mgpuPhysicalDeviceEnumerateSurfacePresentModes(MGPUPhysicalDevice physical_device, MGPUSurface surface, uint32_t* present_mode_count, MGPUPresentMode* present_modes);
MGPUResult mgpuPhysicalDeviceCreateDevice(MGPUPhysicalDevice physical_device, MGPUDevice* device);

// MGPUDevice methods
MGPUQueue mgpuDeviceGetQueue(MGPUDevice device, MGPUQueueType queue_type);
MGPUResult mgpuDeviceCreateBuffer(MGPUDevice device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer);
MGPUResult mgpuDeviceCreateTexture(MGPUDevice device, const MGPUTextureCreateInfo* create_info, MGPUTexture* texture);
MGPUResult mgpuDeviceCreateCommandList(MGPUDevice device, MGPUCommandList* command_list);
MGPUResult mgpuDeviceCreateSwapChain(MGPUDevice device, const MGPUSwapChainCreateInfo* create_info, MGPUSwapChain* swap_chain);
void mgpuDeviceDestroy(MGPUDevice device);

// MGPUQueue methods
MGPUResult mgpuQueueSubmitCommandList(MGPUQueue queue, MGPUCommandList command_list);
MGPUResult mgpuQueueFlush(MGPUQueue queue);

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

// MGPUCommandList methods
MGPUResult mgpuCommandListClear(MGPUCommandList command_list);
void mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, const MGPURenderPassBeginInfo* begin_info);
void mgpuCommandListCmdEndRenderPass(MGPUCommandList command_list);
void mgpuCommandListDestroy(MGPUCommandList command_list);

// MGPUSurface methods
void mgpuSurfaceDestroy(MGPUSurface surface);

// MGPUSwapChain methods
MGPUResult mgpuSwapChainEnumerateTextures(MGPUSwapChain swap_chain, uint32_t* texture_count, MGPUTexture* textures);
MGPUResult mgpuSwapChainAcquireNextTexture(MGPUSwapChain swap_chain, uint32_t* texture_index);
MGPUResult mgpuSwapChainPresent(MGPUSwapChain swap_chain);
void mgpuSwapChainDestroy(MGPUSwapChain swap_chain);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // MGPU_H
