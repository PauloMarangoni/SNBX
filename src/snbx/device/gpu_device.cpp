#include "gpu_device.hpp"
#include "gpu_device_api.hpp"

GPUDeviceAPI device_api{};

void d3d12_device_register(GPUDeviceAPI& gpu_device_api);
void vulkan_device_register(GPUDeviceAPI& gpu_device_api);

GPUResult gpu_device_init() {
   // d3d12_device_register(device_api);
    vulkan_device_register(device_api);
    return device_api.init();
}

GPUSwapchain gpu_device_create_swapchain(const SwapchainCreation& swapchain_creation, Window* window) {
    return device_api.create_swapchain(swapchain_creation, window);
}

GPUPipelineState gpu_device_create_graphics_pipeline(const GraphicsPipelineCreation& graphics_pipeline_creation) {
    return device_api.create_graphics_pipeline(graphics_pipeline_creation);
}

GPURenderPass gpu_device_create_render_pass(const RenderPassCreation& render_pass_creation) {
    return device_api.create_render_pass(render_pass_creation);
}

GPUTexture gpu_device_create_texture(const TextureCreation& texture_creation) {
    return device_api.create_texture(texture_creation);
}

void gpu_device_begin_render_pass(const GPUCommands& cmd, const BeginRenderPassInfo& begin_render_pass_info) {
    device_api.begin_render_pass(cmd, begin_render_pass_info);
}

void gpu_device_end_render_pass(const GPUCommands& cmd) {
    device_api.end_render_pass(cmd);
}

void gpu_device_set_viewport(const GPUCommands& cmd, const ViewportInfo& viewport_info) {
    device_api.set_viewport(cmd, viewport_info);
}

void gpu_device_set_scissor(const GPUCommands& cmd, const Rect& rect) {
    device_api.set_scissor(cmd, rect);
}

GPUCommands gpu_device_begin_frame() {
    return device_api.begin_frame();
}

void gpu_device_end_frame(const GPUSwapchain& swapchain) {
    device_api.end_frame(swapchain);
}

void gpu_device_destroy_swapchain(const GPUSwapchain& swapchain) {
    device_api.destroy_swapchain(swapchain);
}

void gpu_device_destroy_graphics_pipeline(const GPUPipelineState& gpu_pipeline_state) {
    device_api.destroy_graphics_pipeline(gpu_pipeline_state);
}

void gpu_device_destroy_render_pass(const GPURenderPass& gpu_render_pass) {
    device_api.destroy_render_pass(gpu_render_pass);
}

void gpu_device_destroy_texture(const GPUTexture& gpu_texture) {
    device_api.destroy_texture(gpu_texture);
}

void gpu_device_wait() {
    device_api.wait();
}

void gpu_device_shutdown() {
    device_api.shutdown();
}
