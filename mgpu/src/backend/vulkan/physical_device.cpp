
#include <atom/panic.hpp>
#include <algorithm>
#include <cstring>
#include <vector>

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "common/limits.hpp"
#include "conversion.hpp"
#include "device.hpp"
#include "physical_device.hpp"
#include "surface.hpp"

namespace mgpu::vulkan {

PhysicalDevice::PhysicalDevice(VkInstance vk_instance, VulkanPhysicalDevice& vk_physical_device, const QueueFamilyIndices& queue_family_indices)
    : PhysicalDeviceBase{GetInfo(vk_physical_device)}
    , m_vk_instance{vk_instance}
    , m_vk_physical_device{vk_physical_device}
    , m_queue_family_indices{queue_family_indices} {
}

Result<MGPUSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(mgpu::SurfaceBase* surface) {
  VkSurfaceCapabilitiesKHR vk_surface_capabilities{};
  MGPU_VK_FORWARD_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vk_physical_device.Handle(), ((Surface*)surface)->Handle(), &vk_surface_capabilities));

  return MGPUSurfaceCapabilities{
    .min_texture_count = vk_surface_capabilities.minImageCount,
    .max_texture_count = vk_surface_capabilities.maxImageCount,
    .current_extent = {
      .width = vk_surface_capabilities.currentExtent.width,
      .height = vk_surface_capabilities.currentExtent.height,
    },
    .min_texture_extent = {
      .width = vk_surface_capabilities.minImageExtent.width,
      .height = vk_surface_capabilities.minImageExtent.height,
    },
    .max_texture_extent = {
      .width = vk_surface_capabilities.maxImageExtent.width,
      .height = vk_surface_capabilities.maxImageExtent.height,
    },
    .supported_usage = VkImageUsageToMGPUTextureUsage(vk_surface_capabilities.supportedUsageFlags)
  };
}

Result<std::vector<MGPUSurfaceFormat>> PhysicalDevice::EnumerateSurfaceFormats(mgpu::SurfaceBase* surface) {
  uint32_t vk_surface_format_count{};
  std::vector<VkSurfaceFormatKHR> vk_surface_formats{};

  MGPU_VK_FORWARD_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device.Handle(), ((Surface*)surface)->Handle(), &vk_surface_format_count, nullptr));
  vk_surface_formats.resize(vk_surface_format_count);
  MGPU_VK_FORWARD_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device.Handle(), ((Surface*)surface)->Handle(), &vk_surface_format_count, vk_surface_formats.data()));

  std::vector<MGPUSurfaceFormat> mgpu_surface_formats{};

  for(const VkSurfaceFormatKHR& vk_surface_format : vk_surface_formats) {
    MGPUTextureFormat mgpu_format;
    MGPUColorSpace mgpu_color_space;

    switch(vk_surface_format.format) {
      case VK_FORMAT_B8G8R8A8_SRGB: mgpu_format = MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB; break;
      default: continue;
    }

    switch(vk_surface_format.colorSpace) {
      case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: mgpu_color_space = MGPU_COLOR_SPACE_SRGB_NONLINEAR; break;
      default: continue;
    }

    mgpu_surface_formats.emplace_back(mgpu_format, mgpu_color_space);
  }

  return mgpu_surface_formats;
}

Result<std::vector<MGPUPresentMode>> PhysicalDevice::EnumerateSurfacePresentModes(mgpu::SurfaceBase* surface) {
  uint32_t vk_present_mode_count{};
  std::vector<VkPresentModeKHR> vk_present_modes{};

  MGPU_VK_FORWARD_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device.Handle(), ((Surface*)surface)->Handle(), &vk_present_mode_count, nullptr));
  vk_present_modes.resize(vk_present_mode_count);
  MGPU_VK_FORWARD_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device.Handle(), ((Surface*)surface)->Handle(), &vk_present_mode_count, vk_present_modes.data()));

  std::vector<MGPUPresentMode> mgpu_present_modes{};

  for(VkPresentModeKHR vk_present_mode : vk_present_modes) {
    switch(vk_present_mode) {
      case VK_PRESENT_MODE_IMMEDIATE_KHR:    mgpu_present_modes.push_back(MGPU_PRESENT_MODE_IMMEDIATE);    break;
      case VK_PRESENT_MODE_MAILBOX_KHR:      mgpu_present_modes.push_back(MGPU_PRESENT_MODE_MAILBOX);      break;
      case VK_PRESENT_MODE_FIFO_KHR:         mgpu_present_modes.push_back(MGPU_PRESENT_MODE_FIFO);         break;
      case VK_PRESENT_MODE_FIFO_RELAXED_KHR: mgpu_present_modes.push_back(MGPU_PRESENT_MODE_FIFO_RELAXED); break;
      default: break;
    }
  }

  return mgpu_present_modes;
}

Result<DeviceBase*> PhysicalDevice::CreateDevice() {
  return Device::Create(m_vk_instance, m_vk_physical_device, m_queue_family_indices, Limits());
}

MGPUPhysicalDeviceInfo PhysicalDevice::GetInfo(VulkanPhysicalDevice& vk_physical_device) {
  static_assert(MGPU_MAX_PHYSICAL_DEVICE_NAME_SIZE <= VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);

  const VkPhysicalDeviceProperties& vk_device_props = vk_physical_device.GetProperties();
  MGPUPhysicalDeviceInfo mgpu_device_info{};

  MGPUPhysicalDeviceType mgpu_physical_device_type;
  switch(vk_device_props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;   break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    mgpu_physical_device_type = MGPU_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;    break;
    default: ATOM_PANIC("unhandled Vulkan physical device type: {}", (int)vk_device_props.deviceType);
  }

  std::strcpy(mgpu_device_info.device_name, vk_device_props.deviceName);
  mgpu_device_info.device_type = mgpu_physical_device_type;
  mgpu_device_info.limits = GetLimits(vk_device_props.limits);
  return mgpu_device_info;
}

MGPUPhysicalDeviceLimits PhysicalDevice::GetLimits(const VkPhysicalDeviceLimits& vk_device_limits) {
  MGPUPhysicalDeviceLimits mgpu_device_limits{};

  // TODO(fleroviux): check if in practice any devices have lower cube texture limit than 2D texture dimension limit.
  mgpu_device_limits.max_texture_dimension_1d = vk_device_limits.maxImageDimension1D;
  mgpu_device_limits.max_texture_dimension_2d = std::min(vk_device_limits.maxImageDimension2D, vk_device_limits.maxImageDimensionCube);
  mgpu_device_limits.max_texture_dimension_3d = vk_device_limits.maxImageDimension3D;
  mgpu_device_limits.max_texture_array_layers = vk_device_limits.maxImageArrayLayers;
  mgpu_device_limits.max_color_attachments = std::min<u32>(vk_device_limits.maxColorAttachments, limits::max_color_attachments);
  mgpu_device_limits.max_attachment_dimension = std::min(vk_device_limits.maxFramebufferWidth, vk_device_limits.maxFramebufferHeight);
  mgpu_device_limits.max_vertex_input_bindings = std::min<u32>(vk_device_limits.maxVertexInputBindings, limits::max_vertex_input_bindings);
  mgpu_device_limits.max_vertex_input_attributes = std::min<u32>(vk_device_limits.maxVertexInputAttributes, limits::max_vertex_input_attributes);
  mgpu_device_limits.max_vertex_input_binding_stride = vk_device_limits.maxVertexInputBindingStride;
  mgpu_device_limits.max_vertex_input_attribute_offset = vk_device_limits.maxVertexInputAttributeOffset;

  return mgpu_device_limits;
}

}  // namespace mgpu::vulkan
