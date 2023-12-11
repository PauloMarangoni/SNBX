#pragma once

#include "snbx/common.hpp"
#include "snbx/device/gpu_types.hpp"

SNBX_API void render_test_init(const UVec2& size);
SNBX_API void render_test_shutdown();
SNBX_API void render_test_render(const GPUCommands& cmds);
SNBX_API void render_test_blit(const GPUCommands& cmds);
