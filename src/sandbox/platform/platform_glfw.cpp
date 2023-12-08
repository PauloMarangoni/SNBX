#include "platform.hpp"
#include "glfw/glfw3.h"


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

void platform_destroy_window(Window *window) {
    glfwDestroyWindow(window->glfwWindow);
    delete window;
}






