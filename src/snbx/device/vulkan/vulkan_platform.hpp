#pragma once

#include "snbx/common.hpp"
#include "volk.h"

struct Window;

void                platform_init_vk();
Span<const char*>   platform_get_required_instance_extensions();
VkResult            platform_create_window_surface(const Window* window, VkInstance instance, VkSurfaceKHR* surface);
bool                platform_get_physical_device_presentation_support(VkInstance instance, VkPhysicalDevice device, u32 queue_family);
