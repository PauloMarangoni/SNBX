#pragma once

#define GPU_HANDLER(StructName) struct StructName {         \
void* handler;                                              \
operator bool() const {return handler != nullptr; }		    \
bool operator==(const StructName& b) const { return this->handler == b.handler; } \
bool operator!=(const StructName& b) const { return this->handler != b.handler; } \
}

GPU_HANDLER(GPUSwapchain);
GPU_HANDLER(GPUCommands);
GPU_HANDLER(GPURenderPass);

struct GPUDeviceInfo {
    bool raytrace_supported{};
    bool bindless_supported{};
    bool multi_draw_indirect_supported{};
};

struct BeginRenderPassInfo {
    UVec2           extent{};
    GPURenderPass   render_pass{};
    GPUSwapchain    swapchain{};
    Span<Vec4>      clear_values{};
    Vec2            depth_stencil{1.0, 0.0};
};

struct SwapchainCreation {
    bool vsync = true;
};

struct ViewportInfo {
    f32 x{};
    f32 y{};
    f32 width{};
    f32 height{};
    f32 min_depth{};
    f32 max_depth{};
};

//enums
enum class GPUResult {
    Success = 0,
    NotSupported = 1,
};
