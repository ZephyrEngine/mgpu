
#include <atom/panic.hpp>

#include "backend/vulkan/platform/physical_device.hpp"
#include "backend/vulkan/platform/surface.hpp"
#include "instance.hpp"
#include "surface.hpp"

namespace mgpu::vulkan {

Instance::Instance(std::unique_ptr<VulkanInstance> vk_instance)
    : m_vk_instance{std::move(vk_instance)} {
  BuildPhysicalDeviceList();
}

Instance::~Instance() {
  for(auto physical_device : m_physical_devices) delete physical_device;
}

Result<InstanceBase*> Instance::Create() {
  const VkApplicationInfo app_info{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = "mgpu Vulkan driver",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "mgpu Vulkan driver",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_2
  };

  std::vector<const char*> vk_required_instance_extensions{VK_KHR_SURFACE_EXTENSION_NAME};
  for(auto instance_extension : PlatformGetSurfaceInstanceExtensions()) {
    vk_required_instance_extensions.push_back(instance_extension);
  }

  std::vector<const char*> vk_required_instance_layers{};

  // Enable validation layers in debug builds
#ifndef NDEBUG
  if(VulkanInstance::QueryInstanceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    vk_required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  Result<std::unique_ptr<VulkanInstance>> vk_instance_result = VulkanInstance::Create(
    app_info, vk_required_instance_extensions, vk_required_instance_layers, true);
  MGPU_FORWARD_ERROR(vk_instance_result.Code());

  return new Instance{vk_instance_result.Unwrap()};
}

Result<std::span<PhysicalDeviceBase* const>> Instance::EnumeratePhysicalDevices() {
  return std::span{(PhysicalDeviceBase* const*)m_physical_devices.data(), m_physical_devices.size()};
}

Result<SurfaceBase*> Instance::CreateSurface(const MGPUSurfaceCreateInfo& create_info) {
  return Surface::Create(m_vk_instance->Handle(), create_info);
}

void Instance::BuildPhysicalDeviceList() {
  for(auto& vk_physical_device : m_vk_instance->EnumeratePhysicalDevices()) {
    // Vulkan may expose the system CPU as a physical device. We're only interested in GPUs though.
    if(!vk_physical_device->IsGPU()) {
      continue;
    }

    // We require VK_KHR_swapchain extensions for presentation
    if(!vk_physical_device->QueryDeviceExtensionSupport("VK_KHR_swapchain")) {
      continue;
    }

    const PhysicalDevice::QueueFamilyIndices queue_family_indices = SelectQueueFamilies(*vk_physical_device);

    // We require at least one graphics+compute queue family capable of presentation.
    if(!queue_family_indices.graphics_and_compute.has_value()) {
      continue;
    }

    m_physical_devices.push_back(new PhysicalDevice{m_vk_instance->Handle(), *vk_physical_device, queue_family_indices});
  }
}

PhysicalDevice::QueueFamilyIndices Instance::SelectQueueFamilies(VulkanPhysicalDevice& vk_physical_device) {
  PhysicalDevice::QueueFamilyIndices queue_family_indices{};

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
    switch(queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
      case VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT: {
        if(PlatformQueryPresentationSupport(vk_physical_device.Handle(), queue_family_index)) {
          queue_family_indices.graphics_and_compute = queue_family_index;
        }
        break;
      }
      case VK_QUEUE_COMPUTE_BIT: {
        // TODO(fleroviux): determine if we want to require presentation support for the dedicated compute queue.
        queue_family_indices.dedicated_compute = queue_family_index;
        break;
      }
    }

    queue_family_index++;
  }

  return queue_family_indices;
}

}  // namespace mgpu::vulkan
