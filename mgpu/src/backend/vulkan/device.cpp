
#define VMA_IMPLEMENTATION

#include <atom/float.hpp>

#include "buffer.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"

namespace mgpu::vulkan {

Device::Device(
  VkDevice vk_device,
  VmaAllocator vma_allocator,
  std::unique_ptr<CommandQueue> command_queue,
  const MGPUPhysicalDeviceLimits& limits
)   : DeviceBase{limits}
    , m_vk_device{vk_device}
    , m_vma_allocator{vma_allocator}
    , m_command_queue{std::move(command_queue)} {
}

Device::~Device() {
  m_command_queue.reset(); // Ensure that command queue is destroyed before the device
  m_deleter_queue.Drain();

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

  Result<VkDevice> vk_device_result = vk_physical_device.CreateLogicalDevice(vk_queue_create_infos, vk_required_device_extensions, vk_required_device_layers);
  MGPU_FORWARD_ERROR(vk_device_result.Code());

  VkDevice vk_device = vk_device_result.Unwrap();

  Result<VmaAllocator> vma_allocator_result = CreateVmaAllocator(vk_instance, vk_physical_device.Handle(), vk_device);
  MGPU_FORWARD_ERROR(vma_allocator_result.Code());

  Result<std::unique_ptr<CommandQueue>> command_queue_result = CommandQueue::Create(vk_device, queue_family_indices);
  MGPU_FORWARD_ERROR(command_queue_result.Code());

  return new Device{vk_device, vma_allocator_result.Unwrap(), command_queue_result.Unwrap(), limits};
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

Result<BufferBase*> Device::CreateBuffer(const MGPUBufferCreateInfo& create_info) {
  return Buffer::Create(this, create_info);
}

Result<TextureBase*> Device::CreateTexture(const MGPUTextureCreateInfo& create_info) {
  return Texture::Create(this, create_info);
}

Result<SwapChainBase*> Device::CreateSwapChain(const MGPUSwapChainCreateInfo& create_info) {
  return SwapChain::Create(this, create_info);
}

MGPUResult Device::SubmitCommandList(const CommandList* command_list) {
  return m_command_queue->SubmitCommandList(command_list);
}

MGPUResult Device::Flush() {
  return m_command_queue->Flush();
}

}  // namespace mgpu::vulkan
