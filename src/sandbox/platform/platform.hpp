#pragma once

#include "sandbox/common.hpp"

struct Window;

struct WindowCreation {
    StringView  title{};
	bool        maximized{};
};

void    platform_init();
Window* platform_create_window(const WindowCreation& windowCreation);
void    platform_process_events();
void*	platform_get_internal_handler(Window* window);
bool    platform_window_request_close(Window* window);
void    platform_destroy_window(Window* window);
