#pragma once
// Includ this file only once in a single compilation.

#define GLFW_INCLUDE_VULKAN  // Signal GLFW to include vulkan headers with itself
#include <GLFW/glfw3.h>

#include <string>

namespace lve {
    class LVEWindow {
       public:
        LVEWindow(int width, int height, std::string name);
        ~LVEWindow();

        // Delete the copy constructor and operator.
        // We don't want to have a dangling pointer to the window.
        LVEWindow(const LVEWindow &) = delete;
        LVEWindow &operator=(const LVEWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); };
        VkExtent2D getExtent() {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        }
        bool wasWindowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow *getGLFWWindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

       private:
        void initWindow();
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;

        GLFWwindow *window;
    };
}  // namespace lve
