#pragma once

#include "gpu_device.hpp"

struct GPUDeviceAPI {
    GPUResult           (*init)();
    GPUSwapchain        (*create_swapchain)(const SwapchainCreation& swapchain_creation, Window* window);
    GPURenderPass       (*create_render_pass)(const RenderPassCreation& render_pass_creation);
    GPUPipelineState    (*create_graphics_pipeline)(const GraphicsPipelineCreation& graphics_pipeline_creation);
    GPUTexture          (*create_texture)(const TextureCreation& texture_creation);

    void                (*begin_render_pass)(const GPUCommands& cmd, const BeginRenderPassInfo& begin_render_pass_info);
    void                (*end_render_pass)(const GPUCommands& cmd);
    void                (*set_viewport)(const GPUCommands& cmd, const ViewportInfo& viewport_info);
    void                (*set_scissor)(const GPUCommands& cmd, const Rect& rect);

    GPUCommands         (*begin_frame)();
    void                (*end_frame)(const GPUSwapchain& swapchain);

    void                (*destroy_swapchain)(const GPUSwapchain& swapchain);
    void                (*destroy_graphics_pipeline)(const GPUPipelineState& gpu_pipeline_state);
    void                (*destroy_render_pass)(const GPURenderPass& gpu_render_pass);
    void                (*destroy_texture)(const GPUTexture& gpu_texture);

    void                (*wait)();
    void                (*shutdown)();
};
