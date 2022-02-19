#include "first_app.hpp"

#include <stdexcept>

namespace lve {
    FirstApp::FirstApp() {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    FirstApp::~FirstApp() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void FirstApp::run() {
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();  // Poll window events
        }
    }

    void FirstApp::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // A pipeline set layout is used to pass data other than vertex data to the vertex and
        // fragment shaders. Like textures.
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        // Push constants are a way to efficiently send a small amound of data to the shader
        // programs.
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(
                lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void FirstApp::createPipeline() {
        PipelineConfigInfo pipelineConfig{};
        LVEPipeline::defaultPipelineConfigInfo(
            pipelineConfig, lveSwapChain.width(), lveSwapChain.height());
        // A render pass describes the structure and format of frame buffer objects and their
        // attachments.
        pipelineConfig.renderPass = lveSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LVEPipeline>(
            lveDevice,
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.vert.spv",
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.frag.spv",
            pipelineConfig);
    }

    void FirstApp::createCommandBuffers() {}
    void FirstApp::drawFrame() {}
}  // namespace lve
