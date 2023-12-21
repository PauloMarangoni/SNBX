// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "Defines.hpp"
#include "SNBX/Platform/PlatformTypes.hpp"
#include "SNBX/Render/RenderTypes.hpp"

namespace SNBX
{

	class SNBX_API Application
	{
		class SNBX_API ApplicationBuilder
		{
		public:

			ApplicationBuilder& Fullscreen(bool fullscreen)
			{
				m_fullscreen = fullscreen;
				return *this;
			}

			ApplicationBuilder& VSync(bool vsync)
			{
				m_fullscreen = vsync;
				return *this;
			}

			ApplicationBuilder& Title(const StringView& title)
			{
				m_titie = title;
				return *this;
			}

			ApplicationBuilder& Size(const UVec2& size)
			{
				m_size = size;
				return *this;
			}

			ApplicationBuilder& Device(GPUDeviceType device)
			{
				m_device = device;
				return *this;
			}

			Application Build();

		private:
			bool          m_fullscreen = false;
			bool          m_vsync      = true;
			String        m_titie      = "SNBX";
			UVec2         m_size       = UVec2(1920, 1080);
			GPUDeviceType m_device     = GPUDeviceType_OpenGL;

			friend class Application;
		};

	public:
		static ApplicationBuilder Builder()
		{
			return {};
		}

		bool Update();
		virtual ~Application();

	private:
		bool m_running = true;
		Window* m_window = nullptr;

		Application(const ApplicationBuilder& builder);
	};

}