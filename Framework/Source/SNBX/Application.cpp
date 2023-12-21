// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "Application.hpp"
#include "SNBX/Platform/Platform.hpp"

namespace SNBX
{
	Application Application::ApplicationBuilder::Build()
	{
		return {*this};
	}

	Application::Application(const Application::ApplicationBuilder& builder)
	{
		Platform::Init();

		if (builder.m_device == GPUDeviceType_OpenGL)
		{
			Platform::InitOpenGL();
		}

		WindowFlags flags = 0;
		if (builder.m_fullscreen)
		{
			flags |= WindowFlags_Maximized;
		}

		m_window = Platform::CreateWindow(builder.m_titie, builder.m_size, flags);
	}

	bool Application::Update()
	{
		Platform::ProcessEvents();
		if (Platform::UserRequestedClose(m_window))
		{
			m_running = false;
		}

		return m_running;
	}

	Application::~Application()
	{

		Platform::DestroyWindow(m_window);
		Platform::Shutdown();
	}

}
