
#ifdef WIN32

#include <windows.h> // must be included before vulkan_win32.h
#include <vulkan/vulkan.h> // must be included before vulkan_win32.h
#include <vulkan/vulkan_win32.h>

#endif

#include "backend/vulkan/lib/vulkan_result.hpp"
#include "surface.hpp"

namespace mgpu::vulkan {

Surface::Surface(VkInstance vk_instance, VkSurfaceKHR vk_surface)
    : m_vk_instance{vk_instance}
    , m_vk_surface{vk_surface} {
}

Surface::~Surface() {
  // TODO(fleroviux): figure out lifetime management for VkSurfaceKHR
  vkDestroySurfaceKHR(m_vk_instance, m_vk_surface, nullptr);
}

Result<SurfaceBase*> Surface::Create(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {

#ifdef WIN32
  const VkWin32SurfaceCreateInfoKHR vk_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .hinstance = create_info.win32.hinstance,
    .hwnd = create_info.win32.hwnd
  };

  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateWin32SurfaceKHR(vk_instance, &vk_surface_create_info, nullptr, &vk_surface));
  return new Surface{vk_instance, vk_surface};
#endif

  return MGPU_INTERNAL_ERROR;
}

}  // namespace mgpu::vulkan
