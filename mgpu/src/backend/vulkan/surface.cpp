
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/platform/surface.hpp"
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
  Result<VkSurfaceKHR> vk_surface_result = PlatformCreateSurface(vk_instance, create_info);
  MGPU_FORWARD_ERROR(vk_surface_result.Code());
  return new Surface{vk_instance, vk_surface_result.Unwrap()};
}

}  // namespace mgpu::vulkan
