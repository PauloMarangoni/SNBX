// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/App.hpp"
#include "SNBX/Render/Device/RenderDevice.hpp"

using namespace SNBX;

RenderPipelineState rasterShader;

void OnBlit(RenderCommands cmd)
{

}

int main(int argc, char** argv)
{
	App::Init(AppCreation{
		.Title = "SceneView",
		.Fullscreen = false,
		.Device = RenderDeviceType_OpenGL,
		.ClearColor = {0.39, 0.58, 0.92, 1},
		.OnBlit = OnBlit
	});

	GraphicsPipelineCreation graphicsPipelineCreation = {
		.shader = "SceneView://RasterShader.hlsl"
	};

	rasterShader = RenderDevice::CreateGraphicsPipelineState(graphicsPipelineCreation);

	while (App::Update()){}

	RenderDevice::DestroyGraphicsPipelineState(rasterShader);

	App::Shutdown();

	return 0;

}