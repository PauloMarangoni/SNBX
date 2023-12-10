#pragma once

#include "snbx/common.hpp"
#include "snbx/device/gpu_types.hpp"

void shader_compiler_init();
bool compile_shader(const StringView& path, const ShaderCreation& shader_creation, Vec<u8>& bytes);