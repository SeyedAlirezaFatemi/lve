#pragma once

#include <memory>
#include <vector>

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_model.hpp"
#include "lve_pipeline.hpp"

namespace lve {
    /**
     * @brief The SimpleRenderSystem manages a pipeline and its layout, and provides the
     * functionality necessary to render a list of game objects.
     */
    class SimpleRenderSystem {
       public:
        SimpleRenderSystem(LVEDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer,
                               std::vector<LVEGameObject> &gameObjects);

       private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        LVEDevice &lveDevice;

        std::unique_ptr<LVEPipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}  // namespace lve
