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
GPU_HANDLER(GPUPipelineState);


enum class CullMode {
    None = 0,
    Front = 1,
    Back = 2
};

enum class PolygonMode {
    Fill = 0,
    Line = 1,
    Point = 2,
};

enum class PrimitiveTopology {
    PointList = 0,
    LineList = 1,
    LineStrip = 2,
    TriangleList = 3,
    TriangleStrip = 4,
    TriangleFan = 5,
    LineListWithAdjacency = 6,
    LineStripWithAdjacency = 7,
    TriangleListWithAdjacency = 8,
    TriangleStripWithAdjacency = 9,
    PatchList = 10,
};

enum class CompareOp {
    Never = 0,
    Less = 1,
    Equal = 2,
    LessOrEqual = 3,
    Greater = 4,
    NotEqual = 5,
    GreaterOrEqual = 6,
    Always = 7
};

enum class ShaderStage
{
    Vertex          = 1 << 0,
    Hull            = 1 << 1,
    Domain          = 1 << 2,
    Geometry        = 1 << 3,
    Fragment        = 1 << 4,
    Compute         = 1 << 5,
    Raygen          = 1 << 6,
    Intersect       = 1 << 7,
    AnyHit          = 1 << 8,
    ClosestHit      = 1 << 9,
    Miss            = 1 << 10,
    Callable        = 1 << 11,
    All             = 1 << 12,
};

enum class ShaderBinTarget
{
    SPIRV       = 0,
    DXIL        = 1
};

struct ShaderCreation
{
    ShaderStage         shader_stage{};
    ShaderBinTarget     shader_bin_target{};
    StringView          entry_point{};
};

struct GraphicsPipelineCreation {
    GPURenderPass       render_pass{};
    GPUSwapchain        swapchain{};
    String              shader{};
    bool                depth_write{false};
    bool                stencil_test{false};
    bool                blend_enabled{false};
    f32                 min_depth_bounds{1.0};
    f32                 max_depth_bounds{0.0};

    CullMode            cull_mode{CullMode::None};
    CompareOp           compare_operator{CompareOp::Always};
    PolygonMode         polygon_mode{PolygonMode::Fill};
    PrimitiveTopology   primitive_topology{PrimitiveTopology::TriangleList};
};


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
