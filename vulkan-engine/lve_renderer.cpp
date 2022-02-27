#include "lve_renderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

    LVERenderer::LVERenderer(LVEWindow& window, LVEDevice& device)
        : lveWindow{window}, lveDevice{device} {
        recreateSwapChain();
        createCommandBuffers();
    }
    LVERenderer::~LVERenderer() { freeCommandBuffers(); }

    void LVERenderer::recreateSwapChain() {
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
            std::shared_ptr<LVESwapChain> oldSwapChain = std::move(lveSwapChain);
            lveSwapChain = std::make_unique<LVESwapChain>(lveDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
                throw std::runtime_error("Swap chain image format has changed!");
            }
        }
    }

    void LVERenderer::createCommandBuffers() {
        // lveSwapChain->imageCount() will likely be either 2 or 3 depending on if the device
        // supports double or triple buffering.
        commandBuffers.resize(LVESwapChain::MAX_FRAMES_IN_FLIGHT);

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

    void LVERenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(lveDevice.device(),
                             lveDevice.getCommandPool(),
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer LVERenderer::beginFrame() {
        assert(!isFrameStarted && "Cannot call beginFrame while already in progress.");
        auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
        // Here we can detect if the swap chain has been resized and decide whether or not it needs
        // to be recreated.
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        isFrameStarted = true;
        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        return commandBuffer;
    }

    void LVERenderer::endFrame() {
        assert(isFrameStarted && "Cannot call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % LVESwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void LVERenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted &&
               "Cannot call beginSwapChainRenderPass while frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() &&
               "Cannot begin render pass on command buffer from a different frame");
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        // Which frame buffer this render pass writes in
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

        // Setup render area
        // The area where the shader loads and stores will take place.
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

        // Set the clear values
        // This corresponds to what we want the initial values of the frame buffer attachments
        // cleared to.
        // Index 0 is the color attachment and index 1 is the depth attachment.
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 0.1f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // VK_SUBPASS_CONTENTS_INLINE signals that the subsequent render pass commands will be
        // directly embedded in the primary command buffer itself and no secondary commands will
        // be used.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void LVERenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted &&
               "Cannot call endSwapChainRenderPass while frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() &&
               "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }
}  // namespace lve
