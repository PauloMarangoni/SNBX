// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "Application.hpp"
#include "SNBX/Platform/Platform.hpp"

namespace SNBX
{
	Application Application::ApplicationBuilder::Build()
	{
		Platform::Init();


		return {};
	}

	bool Application::Update()
	{
		return m_running;
	}

	Application::~Application()
	{
		Platform::Shutdown();
	}
}
