
#pragma once

#include <vulkan/vulkan.h>

#include "backend/surface.hpp"
#include "common/result.hpp"

namespace mgpu::vulkan {

class Surface final : public SurfaceBase {
  public:
   ~Surface() override;

    static Result<SurfaceBase*> Create(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info);

    [[nodiscard]] VkSurfaceKHR Handle() { return m_vk_surface; }

  private:
    Surface(VkInstance vk_instance, VkSurfaceKHR vk_surface);

    VkInstance m_vk_instance;
    VkSurfaceKHR m_vk_surface;

};

}  // namespace mgpu::vulkan
