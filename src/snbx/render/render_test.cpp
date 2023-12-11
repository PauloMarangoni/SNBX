#include "render_test.hpp"

#include "snbx/device/gpu_device.hpp"

GPUTexture       test_render_pass_texture;
GPURenderPass    test_render_pass;
GPUPipelineState test_render_pipeline_state;
UVec2            test_render_size;


void render_test_init(const UVec2& size) {
    test_render_size = size;
    test_render_pass_texture = gpu_device_create_texture(TextureCreation{
        .extent = {size.x, size.y, 1},
        .render_flags = RenderFlags::RenderPass
    });

    AttachmentCreation attachment_creation{
        .texture = test_render_pass_texture
    };

    const RenderPassCreation render_pass_creation{
        .attachments = {&attachment_creation, 1}
    };

    test_render_pass = gpu_device_create_render_pass(render_pass_creation);

    const GraphicsPipelineCreation graphics_pipeline_creation{
        .shader = "shaders/raster_shader.hlsl"
    };
    test_render_pipeline_state = gpu_device_create_graphics_pipeline(graphics_pipeline_creation);

}

void render_test_render(const GPUCommands& cmds) {
    Vec4 clear_color = {0, 0, 0, 1};

    gpu_device_begin_render_pass(cmds, BeginRenderPassInfo{
                                     .extent = {test_render_size},
                                     .render_pass = test_render_pass,
                                     .clear_values = {&clear_color, 1}
                                 });

    gpu_device_end_render_pass(cmds);
}

void render_test_blit(const GPUCommands& cmds) {
}

void render_test_shutdown() {
    gpu_device_destroy_graphics_pipeline(test_render_pipeline_state);
    gpu_device_destroy_render_pass(test_render_pass);
    gpu_device_destroy_texture(test_render_pass_texture);
}
