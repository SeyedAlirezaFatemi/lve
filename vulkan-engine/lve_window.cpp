#include "lve_window.hpp"

namespace lve {
    LVEWindow::LVEWindow(int width, int height, std::string name) : width{width}, height{height}, windowName{name} {
        initWindow();
    }

    LVEWindow::~LVEWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LVEWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Do no create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Disable window resize

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }
} // namespace lve
