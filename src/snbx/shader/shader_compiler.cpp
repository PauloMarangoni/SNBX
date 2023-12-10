#include "snbx/common.hpp"
#include "snbx/platform/platform.hpp"
#include "shader_compiler.hpp"

#if defined(SNBX_WIN)
#include "Windows.h"
#else
#include "dxc/WinAdapter.h"
#endif

#include "dxc/dxcapi.h"

IDxcUtils*      shader_compiler_utils{};
IDxcCompiler3*  shader_compiler_compiler{};

void shader_compiler_init() {
    void* dx_compiler = platform_load_shared_lib("dxcompiler");
    if (dx_compiler) {
        const DxcCreateInstanceProc create_instance = reinterpret_cast<DxcCreateInstanceProc>(platform_get_proc_address(dx_compiler, "DxcCreateInstance"));
        create_instance(CLSID_DxcUtils, IID_PPV_ARGS(&shader_compiler_utils));
        create_instance(CLSID_DxcCompiler, IID_PPV_ARGS(&shader_compiler_compiler));
    }
}

bool compile_shader(const StringView& path, const ShaderCreation& shader_creation, Vec<u8>& bytes) {
    bool shader_compiler_valid = shader_compiler_utils && shader_compiler_compiler;
    if (!shader_compiler_valid) {
        spdlog::error("[ShaderCompiler] DxShaderCompiler not loaded");
        return false;
    }

    return true;
}
