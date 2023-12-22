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
		if (creation.fullscreen)
		{
			flags |= WindowFlags_Maximized;
		}

		appContext.window     = Platform::CreateWindow(creation.title, creation.size, flags);
		appContext.swapchain  = RenderDevice::CreateSwapchain(appContext.window, creation.vsync);
		appContext.clearColor = creation.clearColor;

		Log::Info("{} Initialized", creation.title);
	}

	bool App::Update()
	{
		Platform::ProcessEvents();
		if (Platform::UserRequestedClose(appContext.window))
		{
			appContext.running = false;
		}

		Extent extent = Platform::GetWindowExtent(appContext.window);

		RenderCommands cmd = RenderDevice::BeginFrame();

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

		RenderDevice::EndRenderPass(cmd);

		RenderDevice::EndFrame(appContext.swapchain);

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
