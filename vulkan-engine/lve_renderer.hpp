#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

namespace lve {
    /**
     * @brief The renderer is responsible for managing the swap chain, command buffers, and drawing
     * functionality.
     * We do not have a one-to-one pairing between the swap chain images and the command buffers.
     * The currentImageIndex is the index of the current swap chain image.
     * The currentFrameIndex is the index of the current command buffer.
     */
    class LVERenderer {
       public:
        LVERenderer(LVEWindow &lveWindow, LVEDevice &lveDevice);
        ~LVERenderer();

        LVERenderer(const LVERenderer &) = delete;
        LVERenderer &operator=(const LVERenderer &) = delete;

        bool isFrameInProgress() const { return isFrameStarted; }
        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        // The application needs to access the swap chain render pass to configure pipelines.
        VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }

        // The reason beginFrame and beginSwapChainRenderPass or the endFrame and
        // endSwapChainRenderPass are not combined:
        // We want the application to have control so we can have multiple render passes.

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

       private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        LVEWindow &lveWindow;
        LVEDevice &lveDevice;
        std::unique_ptr<LVESwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};  // [0, MAX_FRAMES_IN_FLIGHT]
        bool isFrameStarted{false};
    };
}  // namespace lve
