
#define VMA_IMPLEMENTATION

#include <algorithm>
#include <atom/logger/sink/console.hpp>
#include <atom/logger/logger.hpp>
#include <atom/float.hpp>
#include <atom/integer.hpp>
#include <atom/panic.hpp>
#include <fstream>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <vk_mem_alloc.h>

#include "shader/triangle.vert.h"
#include "shader/triangle.frag.h"

#undef main

#ifndef NDEBUG
bool enable_validation_layers = true;
#else
bool enable_validation_layers = false;
#endif

static constexpr int k_window_width = 256 * 4;
static constexpr int k_window_height = 192 * 4;

class HelloTriangle {
  public:
    void Run() {
      Setup();
      MainLoop();
      Cleanup();
    }

  private:
    void Setup() {
      m_window = SDL_CreateWindow(
        "Vulkan Playground",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        k_window_width,
        k_window_height,
        SDL_WINDOW_VULKAN
      );

      CreateVkInstance();

      {
        m_vk_physical_device = PickPhysicalDevice();

        if(m_vk_physical_device == VK_NULL_HANDLE) {
          ATOM_PANIC("Failed to find a suitable GPU!");
        }

        VkPhysicalDeviceProperties physical_device_props{};
        vkGetPhysicalDeviceProperties(m_vk_physical_device, &physical_device_props);
        ATOM_INFO("GPU: {}", physical_device_props.deviceName);
      }

      CreateLogicalDevice();
      CreateVMA();
      CreateSurface();
      CreateSwapChain();
      CreateCommandPool();
      CreateCommandBuffer();
      CreateSemaphore();
      CreateFence();
      CreateRenderPass();
      CreateFramebuffers();
      CreateGraphicsPipeline();
      CreateVBO();
    }

    void MainLoop() {
      SDL_Event event{};

      while(true) {
        while(SDL_PollEvent(&event)) {
          if(event.type == SDL_QUIT) {
            return;
          }
        }

        RenderFrame();
      }
    }

    void RenderFrame() {
      u32 image;

      if(vkAcquireNextImageKHR(m_vk_device, m_vk_swap_chain, 0ull, m_vk_semaphore, VK_NULL_HANDLE, &image) != VK_SUCCESS) {
        ATOM_PANIC("Failed to acquire swap chain image");
      }

      // Wait until the command buffer has been processed by the GPU and can be used again.
      vkWaitForFences(m_vk_device, 1u, &m_vk_fence, VK_TRUE, ~0ull);
      vkResetFences(m_vk_device, 1u, &m_vk_fence);

      // Release any commands that were already recorded into the command buffer back to the command pool.
      vkResetCommandBuffer(m_vk_command_buffer, 0);

      // Begin recording commands into the command buffer
      const VkCommandBufferBeginInfo begin_cmd_buffer_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
      };
      vkBeginCommandBuffer(m_vk_command_buffer, &begin_cmd_buffer_info);
      {
        const VkClearValue clear_value{
          .color = VkClearColorValue{
            .float32 = {0.01f, 0.01f, 0.01f, 1.0f}
          }
        };

        const VkRenderPassBeginInfo render_pass_begin_info{
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .pNext = nullptr,
          .renderPass = m_vk_render_pass,
          .framebuffer = m_vk_swap_chain_fbs[image],
          .renderArea = VkRect2D{
            .offset = VkOffset2D{0u, 0u},
            .extent = VkExtent2D{k_window_width, k_window_height}
          },
          .clearValueCount = 1u,
          .pClearValues = &clear_value
        };

        vkCmdBeginRenderPass(m_vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vk_pipeline);
        VkDeviceSize buffer_offset{0u};
        vkCmdBindVertexBuffers(m_vk_command_buffer, 0u, 1u, &m_vbo_vk_buffer, &buffer_offset);
        vkCmdDraw(m_vk_command_buffer, m_vbo_vertex_count, 1u, 0u, 0u);

        vkCmdEndRenderPass(m_vk_command_buffer);
      }
      vkEndCommandBuffer(m_vk_command_buffer);

      // Execute our command buffer once the acquired image is ready and signal the command buffer fence once the command buffer has executed.
      const VkPipelineStageFlags waiting_stages = VK_PIPELINE_STAGE_TRANSFER_BIT; // This should be enough for now.
      const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1u,
        .pWaitSemaphores = &m_vk_semaphore,
        .pWaitDstStageMask = &waiting_stages,
        .commandBufferCount = 1u,
        .pCommandBuffers = &m_vk_command_buffer,
        .signalSemaphoreCount = 0u,
        .pSignalSemaphores = nullptr
      };
      if(vkQueueSubmit(m_vk_graphics_compute_queue, 1u, &submit_info, m_vk_fence) != VK_SUCCESS) {
        ATOM_PANIC("Queue submit failed :c");
      }

      // TODO: do we need to wait for a semaphore?
      const VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 0u,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1u,
        .pSwapchains = &m_vk_swap_chain,
        .pImageIndices = &image,
        .pResults = nullptr
      };
      vkQueuePresentKHR(m_vk_graphics_compute_queue, &present_info);
    }

    void CreateVkInstance() {
      const VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan Playground",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Vulkan Playground Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 0, 0)
      };

      std::vector<const char*> instance_layers{
      };
      {
        if(enable_validation_layers) {
          const auto validation_layer_name = "VK_LAYER_KHRONOS_validation";

          u32 layer_count;
          std::vector<VkLayerProperties> available_layers{};

          vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
          available_layers.resize(layer_count);
          vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

          const auto predicate =
            [&](const VkLayerProperties& layer_props) {
              return std::strcmp(layer_props.layerName, validation_layer_name) == 0; };

          for(auto& prop : available_layers) ATOM_INFO("{}", prop.layerName);

          if(std::ranges::find_if(available_layers, predicate) != available_layers.end()) {
            instance_layers.push_back(validation_layer_name);
          } else {
            ATOM_WARN("Could not enable instance validation layer");
          }
        }
      }

      // Collect all instance extensions that we need
      std::vector<const char*> required_extension_names{};
      {
        uint extension_count;
        SDL_Vulkan_GetInstanceExtensions(m_window, &extension_count, nullptr);
        required_extension_names.resize(extension_count);
        SDL_Vulkan_GetInstanceExtensions(m_window, &extension_count, required_extension_names.data());
      }

      // Validate that all required extensions are present:
      {
        u32 extension_count;
        std::vector<VkExtensionProperties> available_extensions{};

        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        available_extensions.resize(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

        for(const auto required_extension_name : required_extension_names) {
          const auto predicate =
            [&](const VkExtensionProperties& extensions_props) {
              return std::strcmp(extensions_props.extensionName, required_extension_name) == 0; };

          if(std::ranges::find_if(available_extensions, predicate) == available_extensions.end()) {
            ATOM_PANIC("Could not find required Vulkan instance extension: {}", required_extension_name);
          }
        }
      }

      const VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = (u32)instance_layers.size(),
        .ppEnabledLayerNames = instance_layers.data(),
        .enabledExtensionCount = (u32)required_extension_names.size(),
        .ppEnabledExtensionNames = required_extension_names.data()
      };

      if(vkCreateInstance(&create_info, nullptr, &m_vk_instance) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create Vulkan instance!");
      }
    }

    VkPhysicalDevice PickPhysicalDevice() {
      u32 device_count;
      std::vector<VkPhysicalDevice> physical_devices{};

      vkEnumeratePhysicalDevices(m_vk_instance, &device_count, nullptr);
      physical_devices.resize(device_count);
      vkEnumeratePhysicalDevices(m_vk_instance, &device_count, physical_devices.data());

      std::optional<VkPhysicalDevice> discrete_gpu;
      std::optional<VkPhysicalDevice> integrated_gpu;

      for(VkPhysicalDevice physical_device : physical_devices) {
        VkPhysicalDeviceProperties physical_device_props;

        vkGetPhysicalDeviceProperties(physical_device, &physical_device_props);

        // TODO: pick a suitable GPU based on score.
        // TODO: check if the GPU supports all the features that our render backend will need.

        if(physical_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
          discrete_gpu = physical_device;
        }

        if(physical_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
          integrated_gpu = physical_device;
        }
      }

      return discrete_gpu.value_or(integrated_gpu.value_or((VkPhysicalDevice)VK_NULL_HANDLE));
    }

    void CreateLogicalDevice() {
      // This is pretty much the same logic that we also used for instance layers
      std::vector<const char*> device_layers{
      };
      {
        if(enable_validation_layers) {
          const auto validation_layer_name = "VK_LAYER_KHRONOS_validation";

          u32 layer_count;
          std::vector<VkLayerProperties> available_layers{};

          vkEnumerateDeviceLayerProperties(m_vk_physical_device, &layer_count, nullptr);
          available_layers.resize(layer_count);
          vkEnumerateDeviceLayerProperties(m_vk_physical_device, &layer_count, available_layers.data());

          const auto predicate =
            [&](const VkLayerProperties& layer_props) {
              return std::strcmp(layer_props.layerName, validation_layer_name) == 0; };

          for(auto& prop : available_layers) ATOM_INFO("{}", prop.layerName);

          if(std::ranges::find_if(available_layers, predicate) != available_layers.end()) {
            device_layers.push_back(validation_layer_name);
          } else {
            ATOM_WARN("Could not enable device validation layer");
          }
        }
      }

      // TODO: validate that device extensions are present.
      std::vector<const char*> device_extensions{
        "VK_KHR_swapchain"
      };

      std::optional<u32> graphics_plus_compute_queue_family_index;
      std::optional<u32> dedicated_compute_queue_family_index;

      // Figure out what queues we can create
      std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
      {
        u32 queue_family_count;
        std::vector<VkQueueFamilyProperties> queue_family_props{};

        vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &queue_family_count, nullptr);
        queue_family_props.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, &queue_family_count, queue_family_props.data());

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

        for(u32 queue_family_index = 0; queue_family_index < queue_family_count; queue_family_index++) {
          const auto& queue_family = queue_family_props[queue_family_index];
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

          m_present_queue_family_indices.push_back(graphics_plus_compute_queue_family_index.value());
        } else {
          ATOM_PANIC("Physical device does not have any graphics + compute queue");
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

          m_present_queue_family_indices.push_back(dedicated_compute_queue_family_index.value());

          ATOM_INFO("Got an asynchronous compute queue !");
        }
      }

      const VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = (u32)queue_create_infos.size(),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (u32)device_extensions.size(),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = nullptr
      };

      if(vkCreateDevice(m_vk_physical_device, &create_info, nullptr, &m_vk_device) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create a logical device");
      }

      vkGetDeviceQueue(m_vk_device, graphics_plus_compute_queue_family_index.value(), 0u, &m_vk_graphics_compute_queue);

      if(dedicated_compute_queue_family_index.has_value()) {
        VkQueue queue;
        vkGetDeviceQueue(m_vk_device, dedicated_compute_queue_family_index.value(), 0u, &queue);
        m_vk_dedicated_compute_queue = queue;
      }
    }

    void CreateVMA() {
      const VmaAllocatorCreateInfo vma_create_info = {
        .flags = 0,
        .physicalDevice = m_vk_physical_device,
        .device = m_vk_device,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = m_vk_instance,
        .vulkanApiVersion = VK_API_VERSION_1_0,
        .pTypeExternalMemoryHandleTypes = nullptr
      };

      if(vmaCreateAllocator(&vma_create_info, &m_vma_allocator) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create VMA allocator");
      }
    }

    void CreateSurface() {
      if(!SDL_Vulkan_CreateSurface(m_window, m_vk_instance, &m_vk_surface)) {
        ATOM_PANIC("Failed to create a Vulkan surface for the window");
      }
    }

    void CreateSwapChain() {
      // TODO: query for supported swap chain configurations

      const VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = m_vk_surface,
        .minImageCount = 2,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = VkExtent2D{
          .width = k_window_width,
          .height = k_window_height
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, // TODO
        .imageSharingMode = VK_SHARING_MODE_CONCURRENT, // What are the implications of this?
        .queueFamilyIndexCount = (u32)m_present_queue_family_indices.size(),
        .pQueueFamilyIndices = m_present_queue_family_indices.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr
      };

      if(vkCreateSwapchainKHR(m_vk_device, &create_info, nullptr, &m_vk_swap_chain) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create swap chain :c");
      }

      u32 image_count;
      vkGetSwapchainImagesKHR(m_vk_device, m_vk_swap_chain, &image_count, nullptr);
      m_vk_swap_chain_images.resize(image_count);
      vkGetSwapchainImagesKHR(m_vk_device, m_vk_swap_chain, &image_count, m_vk_swap_chain_images.data());

      for(auto image : m_vk_swap_chain_images) {
        const VkImageViewCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = image,
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = VK_FORMAT_B8G8R8A8_SRGB,
          .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
          },
          .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0u,
            .levelCount = 1u,
            .baseArrayLayer = 0u,
            .layerCount = 1u
          }
        };

        VkImageView view;

        if(vkCreateImageView(m_vk_device, &create_info, nullptr, &view) != VK_SUCCESS) {
          ATOM_PANIC("Failed to create image view");
        }

        m_vk_swap_chain_views.push_back(view);
      }

      ATOM_INFO("Swap chain successfully created");
    }

    void CreateCommandPool() {
      const VkCommandPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_present_queue_family_indices[0] // this is a big dodgy
      };

      if(vkCreateCommandPool(m_vk_device, &create_info, nullptr, &m_vk_command_pool) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create command pool");
      }
    }

    void CreateCommandBuffer() {
      const VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_vk_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1u
      };

      if(vkAllocateCommandBuffers(m_vk_device, &alloc_info, &m_vk_command_buffer) != VK_SUCCESS) {
        ATOM_PANIC("Failed to allocate command buffer");
      }
    }

    void CreateSemaphore() {
      const VkSemaphoreCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
      };

      if(vkCreateSemaphore(m_vk_device, &create_info, nullptr, &m_vk_semaphore) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create semaphore");
      }
    }

    void CreateFence() {
      const VkFenceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };

      if(vkCreateFence(m_vk_device, &create_info, nullptr, &m_vk_fence) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create fence");
      }
    }

    void CreateRenderPass() {
      const VkAttachmentDescription attachment_desc{
        .flags = 0,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      }; // TODO: check if a sub pass dependency is necessary (should be fine though)

      const VkAttachmentReference sub_pass_attachment_ref{
        .attachment = 0u,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      };

      const VkSubpassDescription sub_pass_desc{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0u,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1u,
        .pColorAttachments = &sub_pass_attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0u,
        .pPreserveAttachments = nullptr
      };

      const VkRenderPassCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1u,
        .pAttachments = &attachment_desc,
        .subpassCount = 1u,
        .pSubpasses = &sub_pass_desc,
        .dependencyCount = 0u,
        .pDependencies = nullptr
      };

      if(vkCreateRenderPass(m_vk_device, &create_info, nullptr, &m_vk_render_pass) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create render pass");
      }
    }

    void CreateFramebuffers() {
      for(auto view : m_vk_swap_chain_views) {
        const VkFramebufferCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .renderPass = m_vk_render_pass,
          .attachmentCount = 1u,
          .pAttachments = &view,
          .width = k_window_width,
          .height = k_window_height,
          .layers = 1u
        };

        VkFramebuffer framebuffer;

        if(vkCreateFramebuffer(m_vk_device, &create_info, nullptr, &framebuffer) != VK_SUCCESS) {
          ATOM_PANIC("Failed to create framebuffer");
        }

        m_vk_swap_chain_fbs.push_back(framebuffer);
      }
    }

    void CreateGraphicsPipeline() {
      VkShaderModule vert_shader;
      VkShaderModule frag_shader;
      VkShaderModuleCreateInfo vert_create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = sizeof(triangle_vert),
        .pCode = triangle_vert
      };
      VkShaderModuleCreateInfo frag_create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = sizeof(triangle_frag),
        .pCode = triangle_frag
      };

      if(vkCreateShaderModule(m_vk_device, &vert_create_info, nullptr, &vert_shader) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create vertex shader");
      }
      if(vkCreateShaderModule(m_vk_device, &frag_create_info, nullptr, &frag_shader) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create fragment shader");
      }

      const VkPipelineShaderStageCreateInfo shader_stages[2] {
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = vert_shader,
          .pName = "main",
          .pSpecializationInfo = nullptr
        },
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = frag_shader,
          .pName = "main",
          .pSpecializationInfo = nullptr
        }
      };

      const VkPipelineLayoutCreateInfo layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0u,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0u,
        .pPushConstantRanges = nullptr
      };
      VkPipelineLayout layout;
      if(vkCreatePipelineLayout(m_vk_device, &layout_create_info, nullptr, &layout) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create pipeline layout");
      }

      const VkVertexInputBindingDescription vertex_input_binding_desc{
        .binding = 0u,
        .stride = sizeof(f32) * 10,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
      };

      const VkVertexInputAttributeDescription vertex_input_attribute_descs[]{
        // POSITION
        {
          .location = 0u,
          .binding = 0u,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = 0u
        },
        // COLOR
        {
          .location = 1u,
          .binding = 0u,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = sizeof(f32) * 4
        },
        // UV
        {
          .location = 2u,
          .binding = 0u,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = sizeof(f32) * 8
        }
      };

      const VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1u,
        .pVertexBindingDescriptions = &vertex_input_binding_desc,
        .vertexAttributeDescriptionCount = sizeof(vertex_input_attribute_descs) / sizeof(VkVertexInputAttributeDescription),
        .pVertexAttributeDescriptions = vertex_input_attribute_descs
      };

      const VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
      };

      const VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = k_window_width,
        .height = k_window_height,
        .minDepth = 0, // TODO
        .maxDepth = 1
      };
      const VkRect2D scissor{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{0x7FFFFFFFul, 0x7FFFFFFFul}
      };
      const VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1u,
        .pViewports = &viewport,
        .scissorCount = 1u,
        .pScissors = &scissor
      };

      const VkPipelineRasterizationStateCreateInfo rasterization_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
      };

      const VkPipelineMultisampleStateCreateInfo multisample_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
      };

      const VkPipelineColorBlendAttachmentState attachment_blend_state{
        .blendEnable = VK_FALSE,
        .colorWriteMask = 0xF
      };
      const VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_NO_OP,
        .attachmentCount = 1u,
        .pAttachments = &attachment_blend_state,
        .blendConstants = {0, 0, 0, 0}
      };

      const VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2u, // TODO
        .pStages = shader_stages, // TODO
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = nullptr,
        .layout = layout,
        .renderPass = m_vk_render_pass,
        .subpass = 0u,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
      };

      if(vkCreateGraphicsPipelines(m_vk_device, VK_NULL_HANDLE, 1u, &create_info, nullptr, &m_vk_pipeline) != VK_SUCCESS) {
        ATOM_PANIC("Failed to create graphics pipeline, welp")
      }
    }

    void CreateVBO() {
//      const f32 data[] {
//        // POSITION            |  COLOR              |  UV
//        -0.5,  0.5,  0.0, 1.0,   1.0, 0.0, 0.0, 1.0,   0.0, 0.0,
//         0.5,  0.5,  0.0, 1.0,   0.0, 1.0, 0.0, 1.0,   0.0, 0.0,
//         0.0, -0.5,  0.0, 1.0,   0.0, 0.0, 1.0, 1.0,   0.0, 0.0,
//      };

      std::vector<u8> vbo_data{};

      std::ifstream file{"vbo.bin", std::ios::binary};
      if(!file.good()) {
        ATOM_PANIC("failed to open vbo.bin");
      }

      file.seekg(0u, std::ios::end);
      vbo_data.resize(file.tellg());
      file.seekg(0);
      file.read((char*)vbo_data.data(), vbo_data.size());

      const VkBufferCreateInfo vk_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = vbo_data.size(),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
      };

      const VmaAllocationCreateInfo vma_alloc_info{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
      };

      if(vmaCreateBuffer(m_vma_allocator, &vk_create_info, &vma_alloc_info, &m_vbo_vk_buffer, &m_vbo_vma_allocation, nullptr) != VK_SUCCESS) {
        ATOM_PANIC("Failed to allocate VBO :(");
      }

      void* mapped_address;
      if(vmaMapMemory(m_vma_allocator, m_vbo_vma_allocation, &mapped_address) != VK_SUCCESS) {
        ATOM_PANIC("Failed to map VBO :(")
      }

      std::memcpy(mapped_address, (const void*)vbo_data.data(), vbo_data.size());

      // TODO: is flushing necessary?
      vmaFlushAllocation(m_vma_allocator, m_vbo_vma_allocation, 0u, VK_WHOLE_SIZE);

      vmaUnmapMemory(m_vma_allocator, m_vbo_vma_allocation);

      m_vbo_vertex_count = vbo_data.size() / (sizeof(f32) * 10);
    }

    void Cleanup() {
      vkDeviceWaitIdle(m_vk_device);

      vkDestroyBuffer(m_vk_device, m_vbo_vk_buffer, nullptr);
      vmaFreeMemory(m_vma_allocator, m_vbo_vma_allocation);
      vkDestroyPipeline(m_vk_device, m_vk_pipeline, nullptr);
      for(auto fb : m_vk_swap_chain_fbs) vkDestroyFramebuffer(m_vk_device, fb, nullptr);
      vkDestroyRenderPass(m_vk_device, m_vk_render_pass, nullptr);
      vkDestroyFence(m_vk_device, m_vk_fence, nullptr);
      vkDestroySemaphore(m_vk_device, m_vk_semaphore, nullptr);
      vkFreeCommandBuffers(m_vk_device, m_vk_command_pool, 1u, &m_vk_command_buffer);
      vkDestroyCommandPool(m_vk_device, m_vk_command_pool, nullptr);
      for(auto view : m_vk_swap_chain_views) vkDestroyImageView(m_vk_device, view, nullptr);
      vkDestroySwapchainKHR(m_vk_device, m_vk_swap_chain, nullptr);
      vkDestroySurfaceKHR(m_vk_instance, m_vk_surface, nullptr);
      vmaDestroyAllocator(m_vma_allocator);
      vkDestroyDevice(m_vk_device, nullptr);
      vkDestroyInstance(m_vk_instance, nullptr);
      SDL_DestroyWindow(m_window);
    }

    SDL_Window* m_window{};
    VkInstance m_vk_instance{};
    VkPhysicalDevice m_vk_physical_device{};
    VkDevice m_vk_device{};
    VkQueue m_vk_graphics_compute_queue{};
    std::vector<u32> m_present_queue_family_indices{};
    std::optional<VkQueue> m_vk_dedicated_compute_queue{};
    VmaAllocator m_vma_allocator{};
    VkSurfaceKHR m_vk_surface{VK_NULL_HANDLE};
    VkSwapchainKHR m_vk_swap_chain{};
    std::vector<VkImage> m_vk_swap_chain_images{};
    std::vector<VkImageView> m_vk_swap_chain_views{};
    VkCommandPool m_vk_command_pool{};
    VkCommandBuffer m_vk_command_buffer{};
    VkSemaphore m_vk_semaphore{};
    VkFence m_vk_fence{};
    VkRenderPass m_vk_render_pass{};
    std::vector<VkFramebuffer> m_vk_swap_chain_fbs{};
    VkPipeline m_vk_pipeline{};

    VkBuffer m_vbo_vk_buffer{};
    VmaAllocation m_vbo_vma_allocation{};
    u32 m_vbo_vertex_count{};
};

int main() {
  atom::get_logger().InstallSink(std::make_unique<atom::LoggerConsoleSink>());

  HelloTriangle{}.Run();
  return 0;
}
