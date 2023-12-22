// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/App.hpp"

using namespace SNBX;

int main(int argc, char** argv)
{
	App::Init(AppCreation{
		.title = "SceneView",
		.fullscreen = false,
		.device = GPUDeviceType_OpenGL,
		.clearColor = {0.39, 0.58, 0.92, 1}
	});

	while (App::Update()){}

	App::Shutdown();

	return 0;

}