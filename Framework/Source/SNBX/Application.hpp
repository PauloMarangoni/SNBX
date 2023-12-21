// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "Defines.hpp"

namespace SNBX
{

	class SNBX_API Application
	{
		class ApplicationBuilder
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

			Application Build();

		private:
			bool   m_fullscreen = false;
			bool   m_vsync      = true;
			String m_titie      = "SNBX";

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

	};

}