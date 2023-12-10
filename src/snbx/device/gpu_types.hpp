#pragma once

#define GPU_HANDLER(StructName) struct StructName {         \
void* handler;                                              \
operator bool() const {return handler != nullptr; }		    \
bool operator==(const StructName& b) const { return this->handler == b.handler; } \
bool operator!=(const StructName& b) const { return this->handler != b.handler; } \
}

GPU_HANDLER(GPUSwapchain);

struct GPUDeviceInfo {
    bool raytrace_supported{};
    bool bindless_supported{};
    bool multi_draw_indirect_supported{};
};

struct SwapchainCreation {
    bool vsync = true;
};

//enums
enum class GPUResult {
    Success = 0,
    NotSupported = 1,
};
