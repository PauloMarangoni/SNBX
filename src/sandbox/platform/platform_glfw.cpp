#include "platform.hpp"
#include "glfw/glfw3.h"

#if SNBX_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "glfw/glfw3native.h"

struct Window {
    GLFWwindow *glfwWindow;
};


void platform_init() {

    if (!glfwInit()) {
        spdlog::error("glfw failed to initialize");
        return;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}


Window *platform_create_window(const WindowCreation &windowCreation) {

    Window *window = new Window;

    glfwWindowHint(GLFW_MAXIMIZED, windowCreation.maximized);

    window->glfwWindow = glfwCreateWindow(800, 600, windowCreation.title.data(), nullptr, nullptr);
    glfwShowWindow(window->glfwWindow);

    return window;
}

void platform_process_events() {
    glfwPollEvents();
}

bool platform_window_request_close(Window *window) {
    return glfwWindowShouldClose(window->glfwWindow);
}

void* platform_get_internal_handler(Window* window) {
#if SNBX_WIN
    return glfwGetWin32Window(window->glfwWindow);
#endif;
    SNBX_ASSERT(false, "Not implemented");
}

inline UVec2 platform_window_get_size(Window* window) {
    i32 width, height;
    glfwGetWindowSize(window->glfwWindow, &width, &height);
    return {width, height};
}


void platform_destroy_window(Window *window) {
    glfwDestroyWindow(window->glfwWindow);
    delete window;
}