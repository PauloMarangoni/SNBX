// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/Application.hpp"

using namespace SNBX;

int main(int argc, char** argv)
{
	Application app = Application::Builder()
		.Title("SceneView")
		.Fullscreen(true)
		.Build();

	while(app.Update())
	{
		//TODO
	}

	return 0;

}