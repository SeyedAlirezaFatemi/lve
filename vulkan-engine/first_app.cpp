#include "first_app.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {
    FirstApp::FirstApp() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
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

    void FirstApp::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        // While the window has at least one sizeless dimension, the program will pause and wait.
        // E.g. during minimization.
        while (extent.width == 0 || extent.height == 0) {
            extent = lveWindow.getExtent();
            glfwWaitEvents();
        }

        // Wait until the current swap chain is no longer being used before we create the new swap
        // chain.
        vkDeviceWaitIdle(lveDevice.device());

        if (lveSwapChain == nullptr) {
            lveSwapChain = std::make_unique<LVESwapChain>(lveDevice, extent);
        } else {
            lveSwapChain =
                std::make_unique<LVESwapChain>(lveDevice, extent, std::move(lveSwapChain));
            if (lveSwapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        // Pipeline depends on the swap chain's render pass.
        // Possible optimization: Do no recreate the pipeline if render pass is compatible.
        createPipeline();
    }

    void FirstApp::createPipeline() {
        assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LVEPipeline::defaultPipelineConfigInfo(pipelineConfig);
        // A render pass describes the structure and format of frame buffer objects and their
        // attachments.
        pipelineConfig.renderPass = lveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LVEPipeline>(
            lveDevice,
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.vert.spv",
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.frag.spv",
            pipelineConfig);
    }

    void FirstApp::createCommandBuffers() {
        // lveSwapChain->imageCount() will likely be either 2 or 3 depending on if the device
        // supports double or triple buffering. Each command buffer is going to draw to a different
        // frame buffer.
        commandBuffers.resize(lveSwapChain->imageCount());

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
    }

    void FirstApp::freeCommandBuffers() {
        vkFreeCommandBuffers(lveDevice.device(),
                             lveDevice.getCommandPool(),
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    void FirstApp::recordCommandBuffer(int imageIndex) {
        // We need to record our draw commands to each command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        // Which frame buffer this render pass writes in
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

        // Setup render area
        // The area where the shader loads and stores will take place.
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

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
        vkCmdBeginRenderPass(
            commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Configure the dynamic viewport and scissor.
        // Viewport: Describes the transformation between the pipeline's output and the target
        // image.
        /* Viewport Transformation
                          x                                            x
             ---------------------------->                ---------------------------->
         (-1,-1)                        (1,-1)         (0,0)
            +-----------------------------+              +-----------------------------+
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
        y | |                             |  ------> y | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          v |                             |            v |                             |
            +-----------------------------+              +-----------------------------+
         (-1,1)                         (1,1)                                    (width,height)
         */
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        // For example, if we set the height to height*0.5f, the image would squashed into the top
        // half.
        viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        // Depth range for the viewport
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        // Scissor
        // Like viewport, but instead of squashing the triangle, it cuts it.
        // For example, if we set the height to height*0.5f, the bottom half of the image would be
        // cut.
        VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        lvePipeline->bind(commandBuffers[imageIndex]);
        lveModel->bind(commandBuffers[imageIndex]);
        lveModel->draw(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }

    void FirstApp::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain->acquireNextImage(&imageIndex);
        // Here we can detect if the swap chain has been resized and decide whether or not it needs
        // to be recreated.
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        recordCommandBuffer(imageIndex);
        result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image");
        }
    }
}  // namespace lve
