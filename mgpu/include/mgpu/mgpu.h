
#ifndef MGPU_H
#define MGPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
typedef struct MGPUSamplerImpl* MGPUSampler;
typedef struct MGPUResourceSetLayoutImpl* MGPUResourceSetLayout;
typedef struct MGPUResourceSetImpl* MGPUResourceSet;
typedef struct MGPUShaderModuleImpl* MGPUShaderModule;
typedef struct MGPUShaderProgramImpl* MGPUShaderProgram;
typedef struct MGPURasterizerStateImpl* MGPURasterizerState;
typedef struct MGPUInputAssemblyStateImpl* MGPUInputAssemblyState;
typedef struct MGPUColorBlendStateImpl* MGPUColorBlendState;
typedef struct MGPUVertexInputStateImpl* MGPUVertexInputState;
typedef struct MGPUDepthStencilStateImpl* MGPUDepthStencilState;
typedef struct MGPUCommandListImpl* MGPUCommandList;
typedef struct MGPURenderCommandEncoderImpl* MGPURenderCommandEncoder;
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
  MGPU_BUFFER_INCOMPATIBLE = 8,
  MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE = 9,
  MGPU_INCOMPATIBLE_TEXTURE_FORMAT = 10,
  MGPU_INCOMPATIBLE_TEXTURE_USAGE  = 11,
  MGPU_INCOMPATIBLE_TEXTURE_ASPECT = 12,
  MGPU_NOT_READY = 13,
  MGPU_BAD_COMMAND_LIST = 14,
  MGPU_INVALID_ARGUMENT = 15
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
  MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB = 0,
  MGPU_TEXTURE_FORMAT_DEPTH_F32 = 1
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

// TODO(fleroviux): expose cubic filtering?
typedef enum MGPUTextureFilter {
  MGPU_TEXTURE_FILTER_NEAREST = 0,
  MGPU_TEXTURE_FILTER_LINEAR = 1
} MGPUTextureFilter;

typedef enum MGPUSamplerAddressMode {
  MGPU_SAMPLER_ADDRESS_MODE_REPEAT = 0,
  MGPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
  MGPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2
} MGPUSamplerRepeatMode;

typedef enum MGPUResourceBindingType {
  MGPU_RESOURCE_BINDING_TYPE_SAMPLER = 0,
  MGPU_RESOURCE_BINDING_TYPE_TEXTURE_AND_SAMPLER = 1,
  MGPU_RESOURCE_BINDING_TYPE_SAMPLED_TEXTURE = 2,
  MGPU_RESOURCE_BINDING_TYPE_STORAGE_TEXTURE = 3,
  MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER = 4,
  MGPU_RESOURCE_BINDING_TYPE_STORAGE_BUFFER = 5,
//  MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC = 6,
//  MGPU_RESOURCE_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC = 7
} MGPUResourceBindingType;

typedef enum MGPUShaderStageBits {
  MGPU_SHADER_STAGE_VERTEX = 0x00000001,
  MGPU_SHADER_STAGE_TESSELLATION_CONTROL = 0x00000002,
  MGPU_SHADER_STAGE_TESSELLATION_EVALUATION = 0x00000004,
  MGPU_SHADER_STAGE_GEOMETRY = 0x00000008,
  MGPU_SHADER_STAGE_FRAGMENT = 0x00000010,
  MGPU_SHADER_STAGE_COMPUTE = 0x00000020,

  MGPU_SHADER_STAGE_ALL_GRAPHICS = MGPU_SHADER_STAGE_VERTEX | MGPU_SHADER_STAGE_TESSELLATION_CONTROL | MGPU_SHADER_STAGE_TESSELLATION_EVALUATION | MGPU_SHADER_STAGE_GEOMETRY | MGPU_SHADER_STAGE_FRAGMENT,
  MGPU_SHADER_STAGE_ALL = MGPU_SHADER_STAGE_ALL_GRAPHICS | MGPU_SHADER_STAGE_COMPUTE
} MGPUShaderStageBits;

typedef MGPUFlags MGPUShaderStage;

typedef enum MGPUPolygonMode {
  MGPU_POLYGON_MODE_FILL = 0,
  MGPU_POLYGON_MODE_LINE = 1
} MGPUPolygonMode;

typedef enum MGPUCullModeBits {
  MGPU_CULL_MODE_FRONT = 0x00000001,
  MGPU_CULL_MODE_BACK = 0x00000002,
} MGPUCullModeBits;

typedef MGPUFlags MGPUCullMode;

typedef enum MGPUFrontFace {
  MGPU_FRONT_FACE_COUNTER_CLOCKWISE = 0,
  MGPU_FRONT_FACE_CLOCKWISE = 1
} MGPUFrontFace;

typedef enum MGPUPrimitiveTopology {
  MGPU_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
  MGPU_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
  MGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
  MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
  MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
  MGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST = 5
} MGPUPrimitiveTopology;

typedef enum MGPUBlendFactor {
  MGPU_BLEND_FACTOR_ZERO = 0,
  MGPU_BLEND_FACTOR_ONE = 1,
  MGPU_BLEND_FACTOR_SRC_COLOR = 2,
  MGPU_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
  MGPU_BLEND_FACTOR_DST_COLOR = 4,
  MGPU_BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
  MGPU_BLEND_FACTOR_SRC_ALPHA = 6,
  MGPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
  MGPU_BLEND_FACTOR_DST_ALPHA = 8,
  MGPU_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
  MGPU_BLEND_FACTOR_SRC_ALPHA_SATURATE = 10,
  MGPU_BLEND_FACTOR_SRC1_COLOR = 11,
  MGPU_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 12,
  MGPU_BLEND_FACTOR_SRC1_ALPHA = 13,
  MGPU_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 14
} MGPUBlendFactor;

typedef enum MGPUBlendOp {
  MGPU_BLEND_OP_ADD = 0,
  MGPU_BLEND_OP_SUBTRACT = 1,
  MGPU_BLEND_OP_REVERSE_SUBTRACT = 2,
  MGPU_BLEND_OP_MIN = 3,
  MGPU_BLEND_OP_MAX = 4
} MGPUBlendOp;

typedef enum MGPUColorComponentFlagBits {
  MGPU_COLOR_COMPONENT_R = 1,
  MGPU_COLOR_COMPONENT_G = 2,
  MGPU_COLOR_COMPONENT_B = 4,
  MGPU_COLOR_COMPONENT_A = 8,
} MGPUColorComponentFlagBits;

typedef MGPUFlags MGPUColorComponentFlags;

typedef enum MGPUVertexInputRate {
  MGPU_VERTEX_INPUT_RATE_VERTEX = 0,
  MGPU_VERTEX_INPUT_RATE_INSTANCE = 1
} MGPUVertexInputRate;

// TODO(fleroviux): unstub this
typedef enum MGPUVertexFormat {
  MGPU_VERTEX_FORMAT_STUB_XYZW32323232 = 0,
  MGPU_VERTEX_FORMAT_STUB_XYZ323232 = 1,
  MGPU_VERTEX_FORMAT_STUB_XY3232 = 2
} MGPUVertexFormat;

typedef enum MGPUCompareOp {
  MGPU_COMPARE_OP_NEVER = 0,
  MGPU_COMPARE_OP_LESS = 1,
  MGPU_COMPARE_OP_EQUAL = 2,
  MGPU_COMPARE_OP_LESS_OR_EQUAL = 3,
  MGPU_COMPARE_OP_GREATER = 4,
  MGPU_COMPARE_OP_NOT_EQUAL = 5,
  MGPU_COMPARE_OP_GREATER_OR_EQUAL = 6,
  MGPU_COMPARE_OP_ALWAYS = 7
} MGPUCompareOp;

typedef enum MGPUStencilOp {
  MGPU_STENCIL_OP_KEEP = 0,
  MGPU_STENCIL_OP_ZERO = 1,
  MGPU_STENCIL_OP_REPLACE = 2,
  MGPU_STENCIL_OP_INCREMENT_AND_CLAMP = 3,
  MGPU_STENCIL_OP_DECREMENT_AND_CLAMP = 4,
  MGPU_STENCIL_OP_INVERT = 5,
  MGPU_STENCIL_OP_INCREMENT_AND_WRAP = 6,
  MGPU_STENCIL_OP_DECREMENT_AND_WRAP = 7
} MGPUStencilOp;

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

typedef struct MGPUOffset3D {
  int32_t x;
  int32_t y;
  int32_t z;
} MGPUOffset3D;

typedef struct MGPUColor {
  double r;
  double g;
  double b;
  double a;
} MGPUColor;

typedef struct MGPUTextureUploadRegion {
  // TODO(fleroviux): expose the texture aspects to upload to?
  MGPUOffset3D offset;
  MGPUExtent3D extent;
  uint32_t mip_level;
  uint32_t base_array_layer;
  uint32_t array_layer_count;
} MGPUTextureUploadRegion;

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
  MGPUColorSpace color_space;
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

// NOTE(fleroviux): there appears to be some DirectX compatibility stuff (unnormalized_coordinates for example). Do we actually need that?
typedef struct MGPUSamplerCreateInfo {
  MGPUTextureFilter mag_filter;
  MGPUTextureFilter min_filter;
  MGPUTextureFilter mip_filter;
  MGPUSamplerAddressMode address_mode_u;
  MGPUSamplerAddressMode address_mode_v;
  MGPUSamplerAddressMode address_mode_w;
  float mip_lod_bias;
  bool anisotropy_enable;
  float max_anisotropy;
  bool compare_enable;
  MGPUCompareOp compare_op;
  float min_lod;
  float max_lod;
} MGPUSamplerCreateInfo;

typedef struct MGPUResourceSetLayoutBinding {
  uint32_t binding;
  MGPUResourceBindingType type;
  MGPUShaderStage visibility;
} MGPUResourceSetLayoutBinding;

typedef struct MGPUResourceSetLayoutCreateInfo {
  uint32_t binding_count;
  const MGPUResourceSetLayoutBinding* bindings;
} MGPUResourceSetLayoutCreateInfo;

typedef struct MGPUResourceTextureBinding {
  MGPUTextureView texture_view;
  MGPUSampler sampler;
} MGPUResourceTextureBinding;

typedef struct MGPUResourceBufferBinding {
  MGPUBuffer buffer;
  uint64_t offset;
  uint64_t size;
} MGPUResourceBufferBinding;

typedef struct MGPUResourceSetBinding {
  uint32_t binding;
  MGPUResourceBindingType type; // TODO(fleroviux): do we really need this? It's already in the layout
  union {
    MGPUResourceTextureBinding texture;
    MGPUResourceBufferBinding buffer;
  };
} MGPUResourceSetBinding;

typedef struct MGPUResourceSetCreateInfo {
  MGPUResourceSetLayout layout;
  uint32_t binding_count;
  const MGPUResourceSetBinding* bindings;
} MGPUResourceSetCreateInfo;

typedef struct MGPUShaderStageCreateInfo {
  MGPUShaderStageBits stage;
  MGPUShaderModule module;
  const char* entrypoint;
} MGPUShaderStageCreateInfo;

typedef struct MGPUShaderProgramCreateInfo {
  uint32_t shader_stage_count;
  const MGPUShaderStageCreateInfo* shader_stages;
  uint32_t resource_set_count;
  MGPUResourceSetLayout* resource_set_layouts;
} MGPUShaderProgramCreateInfo;

typedef struct MGPURasterizerStateCreateInfo {
  bool depth_clamp_enable;
  bool rasterizer_discard_enable;
  MGPUPolygonMode polygon_mode;
  MGPUCullMode cull_mode;
  MGPUFrontFace front_face;
  bool depth_bias_enable;
  float depth_bias_constant_factor;
  float depth_bias_clamp;
  float depth_bias_slope_factor;
  float line_width;
} MGPURasterizerStateCreateInfo;

typedef struct MGPUInputAssemblyStateCreateInfo {
  MGPUPrimitiveTopology topology;
  bool primitive_restart_enable;
} MGPUInputAssemblyStateCreateInfo;

typedef struct MGPUColorBlendAttachmentState {
  bool blend_enable;
  MGPUBlendFactor src_color_blend_factor;
  MGPUBlendFactor dst_color_blend_factor;
  MGPUBlendOp color_blend_op;
  MGPUBlendFactor src_alpha_blend_factor;
  MGPUBlendFactor dst_alpha_blend_factor;
  MGPUBlendOp alpha_blend_op;
  MGPUColorComponentFlags color_write_mask;
} MGPUColorBlendAttachmentState;

typedef struct MGPUColorBlendStateCreateInfo {
  // Note: Logic Op and Blend Constants omitted, since they appear to be unsupported on Metal (and MoltenVK).
  uint32_t attachment_count;
  const MGPUColorBlendAttachmentState* attachments;
} MGPUColorBlendStateCreateInfo;

typedef struct MGPUVertexBinding {
  uint32_t binding;
  uint32_t stride;
  MGPUVertexInputRate input_rate;
} MGPUVertexBinding;

typedef struct MGPUVertexAttribute {
  uint32_t location;
  uint32_t binding;
  MGPUVertexFormat format;
  uint32_t offset;
} MGPUVertexAttribute;

typedef struct MGPUVertexInputStateCreateInfo {
  uint32_t binding_count;
  const MGPUVertexBinding* bindings;
  uint32_t attribute_count;
  const MGPUVertexAttribute* attributes;
} MGPUVertexInputStateCreateInfo;

typedef struct MGPUStencilFaceState {
  MGPUStencilOp fail_op;
  MGPUStencilOp pass_op;
  MGPUStencilOp depth_fail_op;
  MGPUCompareOp compare_op;
  uint32_t read_mask;
  uint32_t write_mask;

  // TODO(fleroviux): make the reference value dynamic state?
  uint32_t reference;
} MGPUStencilFaceState;

typedef struct MGPUDepthStencilStateCreateInfo {
  bool depth_test_enable;
  bool depth_write_enable;
  MGPUCompareOp depth_compare_op;
  bool stencil_test_enable;
  MGPUStencilFaceState stencil_front;
  MGPUStencilFaceState stencil_back;
} MGPUDepthStencilStateCreateInfo;

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
  uint32_t max_sampler_allocation_count;
  float max_sampler_lod_bias;
  float max_sampler_anisotropy;
  uint32_t max_color_attachments;
  uint32_t max_attachment_dimension;
  uint32_t max_vertex_input_bindings;
  uint32_t max_vertex_input_attributes;
  uint32_t max_vertex_input_binding_stride;
  uint32_t max_vertex_input_attribute_offset;

  // TODO: resource set limits
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
MGPUResult mgpuDeviceCreateSampler(MGPUDevice device, const MGPUSamplerCreateInfo* create_info, MGPUSampler* sampler);
MGPUResult mgpuDeviceCreateResourceSetLayout(MGPUDevice device, const MGPUResourceSetLayoutCreateInfo* create_info, MGPUResourceSetLayout* resource_set_layout);
MGPUResult mgpuDeviceCreateResourceSet(MGPUDevice device, const MGPUResourceSetCreateInfo* create_info, MGPUResourceSet* resource_set);
MGPUResult mgpuDeviceCreateShaderModule(MGPUDevice device, const uint32_t* spirv_code, size_t spirv_byte_size, MGPUShaderModule* shader_module);
MGPUResult mgpuDeviceCreateShaderProgram(MGPUDevice device, const MGPUShaderProgramCreateInfo* create_info, MGPUShaderProgram* shader_program);
MGPUResult mgpuDeviceCreateRasterizerState(MGPUDevice device, const MGPURasterizerStateCreateInfo* create_info, MGPURasterizerState* rasterizer_state);
MGPUResult mgpuDeviceCreateInputAssemblyState(MGPUDevice device, const MGPUInputAssemblyStateCreateInfo* create_info, MGPUInputAssemblyState* input_assembly_state);
MGPUResult mgpuDeviceCreateColorBlendState(MGPUDevice device, const MGPUColorBlendStateCreateInfo* create_info, MGPUColorBlendState* color_blend_state);
MGPUResult mgpuDeviceCreateVertexInputState(MGPUDevice device, const MGPUVertexInputStateCreateInfo* create_info, MGPUVertexInputState* vertex_input_state);
MGPUResult mgpuDeviceCreateDepthStencilState(MGPUDevice device, const MGPUDepthStencilStateCreateInfo* create_info, MGPUDepthStencilState* depth_stencil_state);
MGPUResult mgpuDeviceCreateCommandList(MGPUDevice device, MGPUCommandList* command_list);
MGPUResult mgpuDeviceCreateSwapChain(MGPUDevice device, const MGPUSwapChainCreateInfo* create_info, MGPUSwapChain* swap_chain);
void mgpuDeviceDestroy(MGPUDevice device);

// MGPUQueue methods
MGPUResult mgpuQueueSubmitCommandList(MGPUQueue queue, MGPUCommandList command_list);
MGPUResult mgpuQueueBufferUpload(MGPUQueue queue, MGPUBuffer buffer, uint64_t offset, uint64_t size, const void* data);
MGPUResult mgpuQueueTextureUpload(MGPUQueue queue, MGPUTexture texture, const MGPUTextureUploadRegion* region, const void* data);
MGPUResult mgpuQueueFlush(MGPUQueue queue);

// MGPUBuffer methods
MGPUResult mgpuBufferMap(MGPUBuffer buffer, void** address);
MGPUResult mgpuBufferUnmap(MGPUBuffer buffer);
MGPUResult mgpuBufferFlushRange(MGPUBuffer buffer, uint64_t offset, uint64_t size);
void mgpuBufferDestroy(MGPUBuffer buffer);

// MGPUTexture methods
MGPUResult mgpuTextureCreateView(MGPUTexture texture, const MGPUTextureViewCreateInfo* create_info, MGPUTextureView* texture_view);
void mgpuTextureDestroy(MGPUTexture texture);

// MGPUSampler methods
void mgpuSamplerDestroy(MGPUSampler sampler);

// MGPUTextureView methods
void mgpuTextureViewDestroy(MGPUTextureView texture_view);

// MGPUResourceSetLayout methods
void mgpuResourceSetLayoutDestroy(MGPUResourceSetLayout resource_set_layout);

// MGPUResourceSet methods
void mgpuResourceSetDestroy(MGPUResourceSet resource_set);

// MGPUShaderModule methods
void mgpuShaderModuleDestroy(MGPUShaderModule shader_module);

// MGPUShaderProgram methods
void mgpuShaderProgramDestroy(MGPUShaderProgram shader_program);

// MGPURasterizerState methods
void mgpuRasterizerStateDestroy(MGPURasterizerState rasterizer_state);

// MGPUInputAssemblyState methods
void mgpuInputAssemblyStateDestroy(MGPUInputAssemblyState input_assembly_state);

// MGPUColorBlendState methods
void mgpuColorBlendStateDestroy(MGPUColorBlendState color_blend_state);

// MGPUVertexInputState methods
void mgpuVertexInputStateDestroy(MGPUVertexInputState vertex_input_state);

// MGPUDepthStencilState methods
void mgpuDepthStencilStateDestroy(MGPUDepthStencilState depth_stencil_state);

// MGPUCommandList methods
MGPUResult mgpuCommandListClear(MGPUCommandList command_list);
MGPURenderCommandEncoder mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, const MGPURenderPassBeginInfo* begin_info);
void mgpuCommandListDestroy(MGPUCommandList command_list);

// MGPURenderCommandEncoder methods
void mgpuRenderCommandEncoderCmdUseShaderProgram(MGPURenderCommandEncoder render_command_encoder, MGPUShaderProgram shader_program);
void mgpuRenderCommandEncoderCmdUseRasterizerState(MGPURenderCommandEncoder render_command_encoder, MGPURasterizerState rasterizer_state);
void mgpuRenderCommandEncoderCmdUseInputAssemblyState(MGPURenderCommandEncoder render_command_encoder, MGPUInputAssemblyState input_assembly_state);
void mgpuRenderCommandEncoderCmdUseColorBlendState(MGPURenderCommandEncoder render_command_encoder, MGPUColorBlendState color_blend_state);
void mgpuRenderCommandEncoderCmdUseVertexInputState(MGPURenderCommandEncoder render_command_encoder, MGPUVertexInputState vertex_input_state);
void mgpuRenderCommandEncoderCmdUseDepthStencilState(MGPURenderCommandEncoder render_command_encoder, MGPUDepthStencilState depth_stencil_state);
void mgpuRenderCommandEncoderCmdSetViewport(MGPURenderCommandEncoder render_command_encoder, float x, float y, float width, float height);
void mgpuRenderCommandEncoderCmdSetScissor(MGPURenderCommandEncoder render_command_encoder, int32_t x, int32_t y, uint32_t width, uint32_t height);
void mgpuRenderCommandEncoderCmdBindVertexBuffer(MGPURenderCommandEncoder render_command_encoder, uint32_t binding, MGPUBuffer buffer, uint64_t buffer_offset);
void mgpuRenderCommandEncoderCmdBindResourceSet(MGPURenderCommandEncoder render_command_encoder, uint32_t index, MGPUResourceSet resource_set);
void mgpuRenderCommandEncoderCmdDraw(MGPURenderCommandEncoder render_command_encoder, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
void mgpuRenderCommandEncoderClose(MGPURenderCommandEncoder render_command_encoder);

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
