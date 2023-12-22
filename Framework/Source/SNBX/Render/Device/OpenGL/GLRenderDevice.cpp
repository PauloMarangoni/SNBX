// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/Render/Device/RenderDeviceAPI.hpp"
#include "GLPlatform.hpp"
#include "GLRenderTypes.hpp"

#ifdef SNBX_DESKTOP
#include "glad/glad.h"
#include "SNBX/Core/Logger.hpp"
#endif

namespace SNBX::GL
{

	struct GLContext
	{
		bool gladLoaded = false;
	};

	GLContext glContext;

	void Init()
	{
		Platform::InitOpenGL();
	}

	void Shutdown()
	{

	}

	RenderSwapchain CreateSwapchain(Window* window, bool vsync)
	{
		Platform::MakeContextCurrent(window);
#ifdef SNBX_DESKTOP
		if (!glContext.gladLoaded && !gladLoadGLLoader((GLADloadproc) Platform::GetProcAddress()))
		{
			Log::Error("[GL] Failed to initialize GLAD");
		}
		glContext.gladLoaded = true;
#endif
		Platform::SetVSync(vsync);
		return {new GLSwapchain{window}};
	}

	void DestroySwapchain(RenderSwapchain swapchain)
	{
		delete static_cast<GLSwapchain*>(swapchain.handler);
	}

	void BeginRenderPass(RenderCommands cmd, const BeginRenderPassInfo& beginRenderPassInfo)
	{
		if (beginRenderPassInfo.swapchain)
		{
			Vec4 cleanColor = beginRenderPassInfo.clearValues[0];
			glClearColor(cleanColor.x, cleanColor.y, cleanColor.z, cleanColor.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else if (beginRenderPassInfo.renderPass)
		{

		}
	}

	void EndRenderPass(RenderCommands cmd)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SetViewport(RenderCommands cmd, const ViewportInfo& viewportInfo)
	{
		glViewport(0, 0, viewportInfo.width, viewportInfo.height);
	}

	void SetScissor(RenderCommands cmd, const Rect& rect)
	{
		glScissor(rect.x, rect.y, rect.width, rect.height);
	}

	RenderCommands BeginFrame()
	{
		return {};
	}

	void EndFrame(RenderSwapchain swapchain)
	{
		Platform::SwapBuffers(static_cast<GLSwapchain*>(swapchain.handler)->window);
	}

}

namespace SNBX
{
	void RegisterOpenGL(RenderDeviceAPI& renderDeviceApi)
	{
		renderDeviceApi.Init             = GL::Init;
		renderDeviceApi.Shutdown         = GL::Shutdown;
		renderDeviceApi.CreateSwapchain  = GL::CreateSwapchain;
		renderDeviceApi.DestroySwapchain = GL::DestroySwapchain;
		renderDeviceApi.BeginRenderPass  = GL::BeginRenderPass;
		renderDeviceApi.EndRenderPass    = GL::EndRenderPass;
		renderDeviceApi.SetViewport      = GL::SetViewport;
		renderDeviceApi.SetScissor       = GL::SetScissor;
		renderDeviceApi.BeginFrame       = GL::BeginFrame;
		renderDeviceApi.EndFrame         = GL::EndFrame;
	}
}
