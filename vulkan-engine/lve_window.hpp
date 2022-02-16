#pragma once
// Includ this file only once in a single compilation.

#define GLFW_INCLUDE_VULKAN // Signal GLFW to include vulkan headers with itself
#include <GLFW/glfw3.h>
#include <string>

namespace lve {
    class LVEWindow {
    public:
        LVEWindow(int width, int height, std::string name);
        ~LVEWindow();
        // Delete the copy constructor and operator. We don't want to have a dangling pointer to the window.
        LVEWindow(const LVEWindow &) = delete;
        LVEWindow &operator=(const LVEWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); };

    private:
        void initWindow();

        const int width;
        const int height;

        std::string windowName;

        GLFWwindow *window;
    };
} // namespace lve
