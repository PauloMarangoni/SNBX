// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "ShaderCompiler.hpp"
#include "SNBX/Platform/Platform.hpp"

#if defined(SNBX_WIN)
#include "Windows.h"
#elif defined(SNBX_UNIX)
#include "dxc/WinAdapter.h"
#endif

#include "dxc/dxcapi.h"

#define SHADER_MODEL "6_5"

#define COMPUTE_SHADER_MODEL    L"cs_" SHADER_MODEL
#define DOMAIN_SHADER_MODEL     L"ds_" SHADER_MODEL
#define GEOMETRY_SHADER_MODEL   L"gs_" SHADER_MODEL
#define HULL_SHADER_MODEL       L"hs_" SHADER_MODEL
#define PIXEL_SHADER_MODELS     L"ps_" SHADER_MODEL
#define VERTER_SHADER_MODEL     L"vs_" SHADER_MODEL
#define LIB_SHADER_MODEL        L"lib_" SHADER_MODEL

namespace SNBX
{

	struct ShaderCompilerContext
	{
		IDxcUtils    * utils{};
		IDxcCompiler3* compiler{};
	};

	ShaderCompilerContext shaderCompilerContext{};

	void ShaderCompiler::Init()
	{
		if (!shaderCompilerContext.compiler && !shaderCompilerContext.utils)
		{
			auto dxCompiler = Platform::LoadSharedLib("dxcompiler");
			if (dxCompiler)
			{
				auto addr = Platform::GetAddress(dxCompiler, "DxcCreateInstance");
				SNBX_ASSERT(addr, "DxcCreateInstance not found");
				auto createInstance =  reinterpret_cast<DxcCreateInstanceProc>(addr);
				createInstance(CLSID_DxcUtils, IID_PPV_ARGS(&shaderCompilerContext.utils));
				createInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&shaderCompilerContext.compiler));
			}
		}
	}

	void ShaderCompiler::Shutdown()
	{
		shaderCompilerContext.compiler->Release();
		shaderCompilerContext.utils->Release();
	}
}

