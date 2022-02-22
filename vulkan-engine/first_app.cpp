#include "first_app.hpp"

#include <array>
#include <stdexcept>

namespace lve {
    FirstApp::FirstApp() {
        loadModels();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    FirstApp::~FirstApp() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

    void FirstApp::run() {
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();  // Poll window events
            drawFrame();
        }
        // CPU will block until all GPU operations have completed
        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadModels() {
        std::vector<LVEModel::Vertex> vertices{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                               {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                               {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        lveModel = std::make_unique<LVEModel>(lveDevice, vertices);
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

    void FirstApp::createCommandBuffers() {
        // lveSwapChain.imageCount() will likely be either 2 or 3 depending on if the device
        // supports double or triple buffering. Each command buffer is going to draw to a different
        // frame buffer.
        commandBuffers.resize(lveSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        // Command pools are opaque objects that command buffer memory is allocated from.
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }

        // We need to record our draw commands to each buffer
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = lveSwapChain.getRenderPass();
            // Which frame buffer this render pass writes in
            renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

            // Setup render area
            // The area where the shader loads and stores will take place.
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

            // Set the clear values
            // This corresponds to what we want the initial values of the frame buffer attachments
            // cleared to.
            // Index 0 is the color attachment and index 1 is the depth attachment.
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 0.1f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            // VK_SUBPASS_CONTENTS_INLINE signals that the subsequent render pass commands will be
            // directly embedded in the primary command buffer itself and no secondary commands will
            // be used.
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            lvePipeline->bind(commandBuffers[i]);
            lveModel->bind(commandBuffers[i]);
            lveModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to record command buffer");
            }
        }
    }
    void FirstApp::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain.acquireNextImage(&imageIndex);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }
        result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image");
        }
    }
}  // namespace lve
