
#define VMA_IMPLEMENTATION

#include <atom/float.hpp>

#include "pipeline_state/color_blend_state.hpp"
#include "pipeline_state/depth_stencil_state.hpp"
#include "pipeline_state/input_assembly_state.hpp"
#include "pipeline_state/rasterizer_state.hpp"
#include "pipeline_state/shader_module.hpp"
#include "pipeline_state/shader_program.hpp"
#include "pipeline_state/vertex_input_state.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "resource_set_layout.hpp"
#include "resource_set.hpp"
#include "sampler.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"

namespace mgpu::vulkan {

Device::Device(
  VkDevice vk_device,
  VmaAllocator vma_allocator,
  const VkPhysicalDeviceFeatures& vk_physical_device_features,
  std::shared_ptr<DeleterQueue> deleter_queue,
  Queues&& queues,
  std::shared_ptr<RenderPassCache> render_pass_cache,
  const MGPUPhysicalDeviceLimits& limits
)   : DeviceBase{limits}
    , m_vk_device{vk_device}
    , m_vma_allocator{vma_allocator}
    , m_vk_physical_device_features{vk_physical_device_features}
    , m_deleter_queue{std::move(deleter_queue)}
    , m_queues{std::move(queues)}
    , m_render_pass_cache{std::move(render_pass_cache)} {
  // TODO(fleroviux): rework architecture to avoid the cyclic dependency between Device and Queue
  m_queues.graphics_compute->SetDevice(this);
  if(m_queues.async_compute) {
    m_queues.async_compute->SetDevice(this);
  }
}

Device::~Device() {
  m_queues = {};               // HACK: ensure that the queues is destroyed before the device
  m_render_pass_cache.reset(); // HACK: ensure that render pass cache is destroyed before the device
  m_deleter_queue->Drain();

  vkDeviceWaitIdle(m_vk_device);
  vmaDestroyAllocator(m_vma_allocator);
  vkDestroyDevice(m_vk_device, nullptr);
}

Result<DeviceBase*> Device::Create(
  VkInstance vk_instance,
  VulkanPhysicalDevice& vk_physical_device,
  const PhysicalDevice::QueueFamilyIndices& queue_family_indices,
  const MGPUPhysicalDeviceLimits& limits
) {
  std::vector<const char*> vk_required_device_extensions{"VK_KHR_swapchain"};
  std::vector<const char*> vk_required_device_layers{};

  // "VK_KHR_portability_subset" device extension must be enabled if it is available. This is required for MoltenVK for example.
  if(vk_physical_device.QueryDeviceExtensionSupport("VK_KHR_portability_subset")) {
    vk_required_device_extensions.push_back("VK_KHR_portability_subset");
  }

  // Enable validation layers in debug builds
#ifndef NDEBUG
  if(vk_physical_device.QueryDeviceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    vk_required_device_layers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  const f32 queue_priority = 0.0f;
  std::vector<VkDeviceQueueCreateInfo> vk_queue_create_infos{};

  if(queue_family_indices.graphics_and_compute.has_value()) {
    vk_queue_create_infos.push_back(VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queue_family_indices.graphics_and_compute.value(),
      .queueCount = 1,
      .pQueuePriorities = &queue_priority
    });
  } else {
    return MGPU_INTERNAL_ERROR;
  }

  if(queue_family_indices.dedicated_compute.has_value()) {
    vk_queue_create_infos.push_back(VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queue_family_indices.dedicated_compute.value(),
      .queueCount = 1,
      .pQueuePriorities = &queue_priority
    });
  }

  VkPhysicalDeviceFeatures vk_physical_device_features{};
  vkGetPhysicalDeviceFeatures(vk_physical_device.Handle(), &vk_physical_device_features);

  Result<VkDevice> vk_device_result = vk_physical_device.CreateLogicalDevice(
    vk_queue_create_infos,
    vk_required_device_extensions,
    vk_required_device_layers,
    &vk_physical_device_features
  );
  MGPU_FORWARD_ERROR(vk_device_result.Code());

  VkDevice vk_device = vk_device_result.Unwrap();

  std::shared_ptr<DeleterQueue> deleter_queue = std::make_shared<DeleterQueue>();
  std::shared_ptr<RenderPassCache> render_pass_cache = std::make_shared<RenderPassCache>(vk_device, deleter_queue);

  Result<VmaAllocator> vma_allocator_result = CreateVmaAllocator(vk_instance, vk_physical_device.Handle(), vk_device);
  MGPU_FORWARD_ERROR(vma_allocator_result.Code()); // TODO(fleroviux): this leaks memory

  std::unique_ptr<Queue> graphics_compute_queue{};
  std::unique_ptr<Queue> async_compute_queue{};

  Result<std::unique_ptr<Queue>> graphics_compute_queue_result = Queue::Create(
    vk_device, queue_family_indices.graphics_and_compute.value(), deleter_queue, render_pass_cache);
  MGPU_FORWARD_ERROR(graphics_compute_queue_result.Code()); // TODO(fleroviux): this leaks memory
  graphics_compute_queue = graphics_compute_queue_result.Unwrap();

  if(queue_family_indices.dedicated_compute.has_value()) {
    Result<std::unique_ptr<Queue>> async_compute_queue_result = Queue::Create(
      vk_device, queue_family_indices.dedicated_compute.value(), deleter_queue, render_pass_cache);
    MGPU_FORWARD_ERROR(async_compute_queue_result.Code()); // TODO(fleroviux): this leaks memory
    async_compute_queue = async_compute_queue_result.Unwrap();
  }

  return new Device{
    vk_device,
    vma_allocator_result.Unwrap(),
    vk_physical_device_features,
    deleter_queue,
    Queues{std::move(graphics_compute_queue), std::move(async_compute_queue)},
    render_pass_cache,
    limits
  };
}

Result<VmaAllocator> Device::CreateVmaAllocator(VkInstance vk_instance, VkPhysicalDevice vk_physical_device, VkDevice vk_device) {
  const VmaAllocatorCreateInfo vma_create_info = {
    .flags = 0,
    .physicalDevice = vk_physical_device,
    .device = vk_device,
    .preferredLargeHeapBlockSize = 0,
    .pAllocationCallbacks = nullptr,
    .pDeviceMemoryCallbacks = nullptr,
    .pHeapSizeLimit = nullptr,
    .pVulkanFunctions = nullptr,
    .instance = vk_instance,
    .vulkanApiVersion = VK_API_VERSION_1_0,
    .pTypeExternalMemoryHandleTypes = nullptr
  };

  VmaAllocator vma_allocator{};
  if(vmaCreateAllocator(&vma_create_info, &vma_allocator) != VK_SUCCESS) {
    return MGPU_INTERNAL_ERROR;
  }
  return vma_allocator;
}

QueueBase* Device::GetQueue(MGPUQueueType queue_type) {
  switch(queue_type) {
    case MGPU_QUEUE_TYPE_GRAPHICS_COMPUTE: return m_queues.graphics_compute.get();
    case MGPU_QUEUE_TYPE_ASYNC_COMPUTE: {
      if(m_queues.async_compute) {
        return m_queues.async_compute.get();
      }
      return m_queues.graphics_compute.get();
    }
  }
  return nullptr;
}

Result<BufferBase*> Device::CreateBuffer(const MGPUBufferCreateInfo& create_info) {
  return Buffer::Create(this, create_info);
}

Result<TextureBase*> Device::CreateTexture(const MGPUTextureCreateInfo& create_info) {
  return Texture::Create(this, create_info);
}

Result<SamplerBase*> Device::CreateSampler(const MGPUSamplerCreateInfo& create_info) {
  return Sampler::Create(this, create_info);
}

Result<ResourceSetLayoutBase*> Device::CreateResourceSetLayout(const MGPUResourceSetLayoutCreateInfo& create_info) {
  return ResourceSetLayout::Create(this, create_info);
}

Result<ResourceSetBase*> Device::CreateResourceSet(const MGPUResourceSetCreateInfo& create_info) {
  return ResourceSet::Create(this, create_info);
}

Result<ShaderModuleBase*> Device::CreateShaderModule(const u32* spirv_code, size_t spirv_byte_size) {
  return ShaderModule::Create(this, spirv_code, spirv_byte_size);
}

Result<ShaderProgramBase*> Device::CreateShaderProgram(const MGPUShaderProgramCreateInfo& create_info) {
  return ShaderProgram::Create(this, create_info);
}

Result<RasterizerStateBase*> Device::CreateRasterizerState(const MGPURasterizerStateCreateInfo& create_info) {
  return new RasterizerState{create_info};
}

Result<InputAssemblyStateBase*> Device::CreateInputAssemblyState(const MGPUInputAssemblyStateCreateInfo& create_info) {
  return new InputAssemblyState{create_info};
}

Result<ColorBlendStateBase*> Device::CreateColorBlendState(const MGPUColorBlendStateCreateInfo& create_info) {
  return new ColorBlendState{create_info};
}

Result<VertexInputStateBase*> Device::CreateVertexInputState(const MGPUVertexInputStateCreateInfo& create_info) {
  return new VertexInputState{create_info};
}

Result<DepthStencilStateBase*> Device::CreateDepthStencilState(const MGPUDepthStencilStateCreateInfo& create_info) {
  return new DepthStencilState{create_info};
}

Result<SwapChainBase*> Device::CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) {
  return SwapChain::Create(this, create_info);
}

}  // namespace mgpu::vulkan
