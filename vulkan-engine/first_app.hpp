#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_window.hpp"

namespace lve {
    class FirstApp {
       public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        void run();

       private:
        LVEWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LVEDevice lveDevice{lveWindow};
        LVEPipeline lvePipeline{lveDevice,
                                "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.vert.spv",
                                "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.frag.spv",
                                LVEPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}  // namespace lve
