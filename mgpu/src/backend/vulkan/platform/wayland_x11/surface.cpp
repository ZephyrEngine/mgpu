
#include <dlfcn.h>
#include <xcb/xcb.h> // Must be included before vulkan_xcb.h
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <vulkan/vulkan_xcb.h>

#include "backend/vulkan/lib/vulkan_instance.hpp"
#include "backend/vulkan/lib/vulkan_result.hpp"
#include "backend/vulkan/platform/surface.hpp"

namespace mgpu::vulkan {

std::vector<const char*> PlatformGetSurfaceInstanceExtensions() {
  std::vector<const char*> extensions{};

  if(VulkanInstance::QueryInstanceExtensionSupport(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) {
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
  }

  if(VulkanInstance::QueryInstanceExtensionSupport(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  }

  return extensions;
}

static Result<VkSurfaceKHR> PlatformCreateSurfaceWayland(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  if(!VulkanInstance::QueryInstanceExtensionSupport(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) {
    return MGPU_INTERNAL_ERROR; // TODO: better error
  }

  const VkWaylandSurfaceCreateInfoKHR vk_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .display = create_info.wayland.display,
    .surface = create_info.wayland.surface
  };
  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateWaylandSurfaceKHR(vk_instance, &vk_surface_create_info, nullptr, &vk_surface));
  return vk_surface;
}

static Result<VkSurfaceKHR> PlatformCreateSurfaceXCB(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  // TODO: should we open the .so.1 or .so file? And can we assume the same name for all platforms?
  // SDL code seems to indicate to use .1 everywhere but OpenBSD
  const auto libx11_xcb1 = dlopen("libX11-xcb.so.1", RTLD_LAZY);
  if(libx11_xcb1 == nullptr) {
    ATOM_PANIC("failed to open libX11-xcb.so.1");
  }

  const auto XGetXCBConnection = (xcb_connection_t* (*)(void*))dlsym(libx11_xcb1, "XGetXCBConnection");
  if(XGetXCBConnection == nullptr) {
    ATOM_PANIC("failed to resolve XGetXCBConnection() from libX11-xcb");
  }

  const auto xcb_connection = XGetXCBConnection(create_info.x11.display);
  if(xcb_connection == nullptr) {
    ATOM_PANIC("failed to get XCB connection from X11 display");
  }

  if(!VulkanInstance::QueryInstanceExtensionSupport(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
    return MGPU_INTERNAL_ERROR; // TODO: better error
  }

  const VkXcbSurfaceCreateInfoKHR vk_surface_create_info{
    .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .connection = xcb_connection,
    .window = (xcb_window_t)create_info.x11.window
  };
  VkSurfaceKHR vk_surface{};
  MGPU_VK_FORWARD_ERROR(vkCreateXcbSurfaceKHR(vk_instance, &vk_surface_create_info, nullptr, &vk_surface));
  return vk_surface;
}

Result<VkSurfaceKHR> PlatformCreateSurface(VkInstance vk_instance, const MGPUSurfaceCreateInfo& create_info) {
  // TODO: use a tagged union to determine which WM subsystem to use instead?

  if(create_info.wayland.display != nullptr) {
    return PlatformCreateSurfaceWayland(vk_instance, create_info);
  }

  if(create_info.x11.display != nullptr) {
    return PlatformCreateSurfaceXCB(vk_instance, create_info);
  }

  return MGPU_INTERNAL_ERROR; // TODO: better error
}

}  // namespace mgpu::vulkan
