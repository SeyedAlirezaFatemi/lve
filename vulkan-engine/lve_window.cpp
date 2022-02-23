#include "lve_window.hpp"

#include <stdexcept>

namespace lve {
    LVEWindow::LVEWindow(int width, int height, std::string name)
        : width{width}, height{height}, windowName{name} {
        initWindow();
    }

    LVEWindow::~LVEWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LVEWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Do no create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);     // Disable or enable window resize

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

        // Pair the window object with this pointer
        glfwSetWindowUserPointer(window, this);
        // Whenever the window is resized, glfw will call the callback with the window pointer and
        // the new width and height.
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    }

    void LVEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        // Create a Vulkan surface for the specified window.
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface.");
        }
    }

    void LVEWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto lveWindow = reinterpret_cast<LVEWindow *>(glfwGetWindowUserPointer(window));
        lveWindow->frameBufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }
}  // namespace lve
