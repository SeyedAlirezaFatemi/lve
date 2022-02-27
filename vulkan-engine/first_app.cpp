#include "first_app.hpp"

#include "simple_render_system.hpp"

// Signal GLM to expect angles to be specified in radians
#define GLM_FORCE_RADIANS
// Signal GLM to expect the depth buffer values to range from 0 to 1. OpenGL is -1 to 1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <array>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>

namespace lve {

    FirstApp::FirstApp() { loadGameObjects(); }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();  // Poll window events

            // beginFrame returns nullptr if the swap chain needs to be recreated
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
        // CPU will block until all GPU operations have completed
        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::vector<LVEModel::Vertex> vertices{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                               {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                               {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        auto lveModel = std::make_shared<LVEModel>(lveDevice, vertices);
        std::vector<glm::vec3> colors{{1.0f, 0.7f, 0.73f},
                                      {1.0f, 0.87f, 0.73f},
                                      {1.0f, 1.0f, 0.73f},
                                      {0.73f, 1.0f, 0.8f},
                                      {0.73, 0.88f, 1.0f}};
        for (auto& color : colors) {
            color = glm::pow(color, glm::vec3{2.2f});
        }
        for (int i = 0; i < 40; i++) {
            auto triangle = LVEGameObject::createGameObject();
            triangle.model = lveModel;
            triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
            triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
            triangle.color = colors[i % colors.size()];
            gameObjects.push_back(std::move(triangle));
        }

        // auto triangle = LVEGameObject::createGameObject();
        // triangle.model = lveModel;
        // triangle.color = {0.1f, 0.8f, 0.1f};
        // triangle.transform2d.translation.x = 0.2f;
        // triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();
        // triangle.transform2d.scale = {2.0f, 0.5f};
        // gameObjects.push_back(std::move(triangle));
    }
}  // namespace lve
