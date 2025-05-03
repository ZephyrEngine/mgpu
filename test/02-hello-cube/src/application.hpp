
#pragma once

#include <atom/float.hpp>
#include <mgpu/mgpu.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <vector>
#undef main

class Application {
  public:
    Application();
   ~Application();

  private:
    void CreateSwapChain();
    void DestroySwapChain();
    void MainLoop();

    SDL_Window* m_sdl_window{};
    MGPUInstance m_mgpu_instance{};
    MGPUPhysicalDevice m_mgpu_physical_device{};
    MGPUSurface m_mgpu_surface{};
    MGPUDevice m_mgpu_device{};
    MGPUSwapChain m_mgpu_swap_chain{};
    std::vector<MGPUTextureView> m_mgpu_swap_chain_texture_views{};
    MGPUTexture m_mgpu_depth_texture{};
    MGPUTextureView m_mgpu_depth_texture_view{};
    MGPUBuffer m_mgpu_vbo{};
    MGPUBuffer m_mgpu_ibo{};
    MGPUBuffer m_mgpu_ubo{};
    MGPUResourceSetLayout m_mgpu_resource_set_layout{};
    MGPUResourceSet m_mgpu_resource_set{};
    MGPUShaderModule m_mgpu_vert_shader{};
    MGPUShaderModule m_mgpu_frag_shader{};
    MGPUShaderProgram m_mgpu_shader_program{};
    MGPUVertexInputState m_mgpu_vertex_input_state{};
    MGPURasterizerState m_mgpu_rasterizer_state{};
    MGPUDepthStencilState m_mgpu_depth_stencil_state{};
    MGPUCommandList m_mgpu_cmd_list{};

    f32 m_aspect_ratio{};
    f32 m_y_rotation{};
};
