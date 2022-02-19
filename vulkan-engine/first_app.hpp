#pragma once

#include <memory>
#include <vector>

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

namespace lve {
    class FirstApp {
       public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        // Delete the copy constructor and operator.
        // That's because we are managing pipelineLayout and commandBuffers.
        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

       private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();
        // Initialized from top to bottom
        LVEWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LVEDevice lveDevice{lveWindow};
        LVESwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
        std::unique_ptr<LVEPipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
    };
}  // namespace lve
