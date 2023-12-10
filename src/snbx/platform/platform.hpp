#pragma once

#include "snbx/common.hpp"

struct Window;

struct WindowCreation {
    StringView  title{};
	bool        maximized{};
};

SNBX_API void    platform_init();
SNBX_API Window* platform_create_window(const WindowCreation& windowCreation);
SNBX_API void    platform_process_events();
SNBX_API void*	 platform_get_internal_handler(Window* window);
SNBX_API bool    platform_window_request_close(Window* window);
SNBX_API UVec2   platform_window_get_size(Window* window);
SNBX_API void    platform_destroy_window(Window* window);
SNBX_API void	 platform_wait_events();