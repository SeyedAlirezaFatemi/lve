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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Disable window resize

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }

    void LVEWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        // Create a Vulkan surface for the specified window.
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface.");
        }
    }
}  // namespace lve
