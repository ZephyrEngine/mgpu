
#include <atom/float.hpp>
#include <fmt/format.h>
#include <optional>

#include "render_device_backend.hpp"

namespace mgpu {

Result<std::unique_ptr<RenderDeviceBackendBase>> VulkanRenderDeviceBackend::Create(SDL_Window* sdl_window) {
  auto vk_instance_result = CreateVulkanInstance(sdl_window);
  MGPU_FORWARD_ERROR(vk_instance_result.Code());

  std::unique_ptr<VulkanInstance> vk_instance = vk_instance_result.Unwrap();

  u32 vk_graphics_compute_queue_family_index{};
  std::vector<u32> vk_present_queue_family_indices{};
  auto vk_device_result = CreateVulkanDevice(PickVulkanPhysicalDevice(vk_instance), vk_graphics_compute_queue_family_index, vk_present_queue_family_indices);
  MGPU_FORWARD_ERROR(vk_device_result.Code());

  VkSurfaceKHR vk_surface{};
  if(!SDL_Vulkan_CreateSurface(sdl_window, vk_instance->Handle(), &vk_surface)) {
    fmt::print("mgpu: Vulkan: failed to create VkSurfaceKHR\n");
    return MGPU_INTERNAL_ERROR;
  }

  return std::unique_ptr<RenderDeviceBackendBase>{new VulkanRenderDeviceBackend{
    std::move(vk_instance),
    vk_device_result.Unwrap(),
    vk_graphics_compute_queue_family_index,
    std::move(vk_present_queue_family_indices),
    vk_surface
  }};
}

Result<std::unique_ptr<VulkanInstance>> VulkanRenderDeviceBackend::CreateVulkanInstance(SDL_Window* sdl_window) {
  const VkApplicationInfo app_info{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = "mgpu Vulkan driver",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "mgpu Vulkan driver",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_MAKE_VERSION(1, 0, 0)
  };

  // Get the list of instance extensions required for SDL2 to create a Vulkan surface.
  std::vector<const char*> required_extension_names{};
  uint extension_count{};
  SDL_Vulkan_GetInstanceExtensions(sdl_window, &extension_count, nullptr);
  required_extension_names.resize(extension_count);
  SDL_Vulkan_GetInstanceExtensions(sdl_window, &extension_count, required_extension_names.data());

  // Enable validation layers in debug builds.
  std::vector<const char*> required_layer_names{};
#ifndef NDEBUG
  if(VulkanInstance::QueryInstanceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    required_layer_names.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  return VulkanInstance::Create(app_info, required_extension_names, required_layer_names);
}

Result<VkDevice> VulkanRenderDeviceBackend::CreateVulkanDevice(
  const VulkanPhysicalDevice* vk_physical_device,
  u32& vk_graphics_compute_queue_family_index,
  std::vector<u32>& vk_present_queue_family_indices
) {
  // Enable validation layers in debug builds.
  std::vector<const char*> required_device_layers{};
#ifndef NDEBUG
  if(vk_physical_device->QueryDeviceLayerSupport("VK_LAYER_KHRONOS_validation")) {
    required_device_layers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif

  std::vector<const char*> required_device_extensions{"VK_KHR_swapchain"};
  for(auto extension_name : required_device_extensions) {
    if(!vk_physical_device->QueryDeviceExtensionSupport(extension_name)) {
      // TODO(fleroviux): let the user know what happened.
      fmt::print("mgpu: Vulkan: missing device extension: {}\n", extension_name);
      return MGPU_INTERNAL_ERROR;
    }
  }

  // Enable Vulkan portability subset (required for MoltenVK support)
  if(vk_physical_device->QueryDeviceExtensionSupport("VK_KHR_portability_subset")) {
    required_device_extensions.push_back("VK_KHR_portability_subset");
  }

  std::optional<u32> graphics_plus_compute_queue_family_index;
  std::optional<u32> dedicated_compute_queue_family_index;

  // Figure out what queues we can create
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
  {
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

    u32 queue_family_index = 0;

    for(const auto& queue_family : vk_physical_device->EnumerateQueueFamilies()) {
      const VkQueueFlags queue_flags = queue_family.queueFlags;

      /**
       * TODO: we require both our graphics + compute queue and our dedicated compute queues to support presentation.
       * But currently we do not do any checking to ensure that this is the case. From the looks of it,
       * it seems like this might require platform dependent code (see vkGetPhysicalDeviceWin32PresentationSupportKHR() for example).
       */
      switch(queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
        case VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT: {
          graphics_plus_compute_queue_family_index = queue_family_index;
          break;
        }
        case VK_QUEUE_COMPUTE_BIT: {
          dedicated_compute_queue_family_index = queue_family_index;
          break;
        }
      }

      queue_family_index++;
    }

    const f32 queue_priority = 0.0f;

    if(graphics_plus_compute_queue_family_index.has_value()) {
      queue_create_infos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphics_plus_compute_queue_family_index.value(),
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
      });

      vk_graphics_compute_queue_family_index = graphics_plus_compute_queue_family_index.value();
      vk_present_queue_family_indices.push_back(vk_graphics_compute_queue_family_index);
    } else {
      fmt::print("mgpu: Vulkan: physical device does not have any graphics + compute queue\n");
      return MGPU_INTERNAL_ERROR;
    }

    if(dedicated_compute_queue_family_index.has_value()) {
      queue_create_infos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = dedicated_compute_queue_family_index.value(),
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
      });

      vk_present_queue_family_indices.push_back(dedicated_compute_queue_family_index.value());
    }
  }

  return vk_physical_device->CreateLogicalDevice(queue_create_infos, required_device_extensions, required_device_layers);
}

const VulkanPhysicalDevice* VulkanRenderDeviceBackend::PickVulkanPhysicalDevice(const std::unique_ptr<VulkanInstance>& vk_instance) {
  VulkanPhysicalDevice* integrated_gpu{};

  for(auto& physical_device : vk_instance->EnumeratePhysicalDevices()) {
    switch(physical_device->GetProperties().deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return physical_device.get();
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: integrated_gpu = physical_device.get(); break;
      default: break;
    }
  }

  return integrated_gpu;
}

VulkanRenderDeviceBackend::VulkanRenderDeviceBackend(
  std::unique_ptr<VulkanInstance> vk_instance,
  VkDevice vk_device,
  u32 vk_graphics_compute_queue_family_index,
  std::vector<u32>&& vk_present_queue_family_indices,
  VkSurfaceKHR vk_surface
)   : m_vk_instance{std::move(vk_instance)}
    , m_vk_device{vk_device}
    , m_vk_graphics_compute_queue_family_index{vk_graphics_compute_queue_family_index}
    , m_vk_present_queue_family_indices{std::move(vk_present_queue_family_indices)}
    , m_vk_surface{vk_surface} {
}

VulkanRenderDeviceBackend::~VulkanRenderDeviceBackend() {
  vkDeviceWaitIdle(m_vk_device);
  vkDestroySurfaceKHR(m_vk_instance->Handle(), m_vk_surface, nullptr);
  vkDestroyDevice(m_vk_device, nullptr);
}

}  // namespace mgpu
