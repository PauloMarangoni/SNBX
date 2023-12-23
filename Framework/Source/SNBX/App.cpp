// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "App.hpp"
#include "SNBX/Platform/Platform.hpp"
#include "SNBX/Core/Logger.hpp"
#include "SNBX/Render/Device/RenderDevice.hpp"
#include "SNBX/Assets/AssetServer.hpp"
#include "SNBX/Render/Shader/ShaderCompiler.hpp"

namespace SNBX
{
	void RegisterShaderAssetHandler();


	struct AppContext
	{
		bool running = true;
		Window* window = nullptr;
		RenderSwapchain swapchain{};
		Vec4            clearColor = Vec4{0, 0, 0, 1};
		FnOnRender      OnRender   = nullptr;
		FnOnUpdate      OnUpdate   = nullptr;
		FnOnBlit        OnBlit     = nullptr;
	};

	AppContext appContext{};

	void App::Init(const AppCreation& creation)
	{
		RegisterShaderAssetHandler();


		Platform::Init();
		RenderDevice::Init(creation);
		ShaderCompiler::Init();
		AssetServer::Init();

		WindowFlags flags = 0;
		if (creation.Fullscreen)
		{
			flags |= WindowFlags_Maximized;
		}

		appContext.window     = Platform::CreateWindow(creation.Title, creation.Size, flags);
		appContext.swapchain  = RenderDevice::CreateSwapchain(appContext.window, creation.Vsync);
		appContext.clearColor = creation.ClearColor;
		appContext.OnUpdate   = creation.OnUpdate;
		appContext.OnRender   = creation.OnRender;
		appContext.OnBlit     = creation.OnBlit;

		Log::Info("{} Initialized", creation.Title);
	}

	bool App::Update()
	{
		Platform::ProcessEvents();
		if (Platform::UserRequestedClose(appContext.window))
		{
			appContext.running = false;
		}

		if (appContext.OnUpdate)
		{
			appContext.OnUpdate(0);
		}

		Extent extent = Platform::GetWindowExtent(appContext.window);

		RenderCommands cmd = RenderDevice::BeginFrame();

		if (appContext.OnRender)
		{
			appContext.OnRender(cmd);
		}

		RenderDevice::BeginRenderPass(cmd, BeginRenderPassInfo{
			.swapchain = appContext.swapchain,
			.clearValues = {&appContext.clearColor, 1}
		});

		ViewportInfo viewportInfo{};
		viewportInfo.x        = 0.;
		viewportInfo.y        = 0.;
		viewportInfo.width    = (f32) extent.width;
		viewportInfo.height   = (f32) extent.height;
		viewportInfo.maxDepth = 0.;
		viewportInfo.minDepth = 1.;
		RenderDevice::SetViewport(cmd, viewportInfo);

		auto scissor = Rect{0, 0, extent.width, extent.height};
		RenderDevice::SetScissor(cmd, scissor);

		if (appContext.OnBlit)
		{
			appContext.OnBlit(cmd);
		}

		RenderDevice::EndRenderPass(cmd);

		RenderDevice::EndFrame(appContext.swapchain);

		return appContext.running;
	}

	RenderSwapchain App::GetSwapchain()
	{
		return appContext.swapchain;
	}

	Window* App::GetWindow()
	{
		return appContext.window;
	}

	bool App::IsRunning()
	{
		return appContext.running;
	}

	void App::Shutdown()
	{
		RenderDevice::DestroySwapchain(appContext.swapchain);
		Platform::DestroyWindow(appContext.window);
		ShaderCompiler::Shutdown();
		AssetServer::Shutdown();
		RenderDevice::Shutdown();
		Platform::Shutdown();
	}


}
