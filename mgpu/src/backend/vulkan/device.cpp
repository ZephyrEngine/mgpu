
#define VMA_IMPLEMENTATION

#include <atom/float.hpp>

#include "buffer.hpp"
#include "device.hpp"

namespace mgpu::vulkan {

Device::Device(VkDevice vk_device, VmaAllocator vma_allocator)
    : m_vk_device{vk_device}
    , m_vma_allocator{vma_allocator} {
}

Device::~Device() {
  vmaDestroyAllocator(m_vma_allocator);
  vkDestroyDevice(m_vk_device, nullptr);
}

Result<DeviceBase*> Device::Create(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device) {
  std::vector<const char*> vk_required_device_extensions{"VK_KHR_swapchain"};
  std::vector<const char*> vk_required_device_layers{};

  // Enable validation layers in debug builds
#ifndef NDEBUG
  if(vk_physical_device.QueryDeviceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    vk_required_device_layers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  const QueueFamilyIndices queue_family_indices = SelectQueueFamilies(vk_physical_device);

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
    // TODO(fleroviux): mgpu::vulkan::Instance should validate that the physical device offers graphics(+compute) queue family.
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

  return new Device{vk_device, vma_allocator_result.Unwrap()};
}

Device::QueueFamilyIndices Device::SelectQueueFamilies(VulkanPhysicalDevice& vk_physical_device) {
  QueueFamilyIndices queue_family_indices{};

  u32 queue_family_index{0};

  /**
   * Info about queues present on the common vendors, gathered from:
   *   http://vulkan.gpuinfo.org/listreports.php
   *
   * Nvidia (up until Pascal (GTX 10XX)):
   *   - 16x graphics + compute + transfer + presentation
   *   -  1x transfer
   *
   * Nvidia (from Pascal (GTX 10XX) onwards):
   *   - 16x graphics + compute + transfer + presentation
   *   -  2x transfer
   *   -  8x compute + transfer + presentation (async compute?)
   *   -  1x transfer + video decode
   *
   * AMD:
   *   Seems to vary quite a bit from GPU to GPU, but usually have at least:
   *   - 1x graphics + compute + transfer + presentation
   *   - 1x compute + transfer + presentation (async compute?)
   *
   * Apple M1 (via MoltenVK):
   *   - 1x graphics + compute + transfer + presentation
   *
   * Intel:
   *   - 1x graphics + compute + transfer + presentation
   *
   * Furthermore the Vulkan spec guarantees that:
   *   - If an implementation exposes any queue family which supports graphics operation, then at least one
   *     queue family of at least one physical device exposed by the implementation must support graphics and compute operations.
   *
   *   - Queues which support graphics or compute commands implicitly always support transfer commands, therefore a
   *     queue family supporting graphics or compute commands might not explicitly report transfer capabilities, despite supporting them.
   *
   * Given this data, we chose to allocate the following queues:
   *   - 1x graphics + compute + transfer + presentation (required)
   *   - 1x compute + transfer + presentation (if present)
   */
  for(const auto& queue_family : vk_physical_device.EnumerateQueueFamilies()) {
    /**
     * TODO: we require both our graphics + compute queue and our dedicated compute queues to support presentation.
     * But currently we do not do any checking to ensure that this is the case. From the looks of it,
     * it seems like this might require platform dependent code (see vkGetPhysicalDeviceWin32PresentationSupportKHR() for example).
     */
    switch(queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
      case VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT: {
        queue_family_indices.graphics_and_compute = queue_family_index;
        break;
      }
      case VK_QUEUE_COMPUTE_BIT: {
        queue_family_indices.dedicated_compute = queue_family_index;
        break;
      }
    }

    queue_family_index++;
  }

  return queue_family_indices;
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
  return Buffer::Create(m_vk_device, m_vma_allocator, create_info);
}

}  // namespace mgpu::vulkan
