#pragma once

#include <memory>
#include <vector>

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"

namespace lve {
    class FirstApp {
       public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

       private:
        void loadGameObjects();

        // Initialized from top to bottom
        LVEWindow lveWindow{WIDTH, HEIGHT, "Vulkan Engine"};
        LVEDevice lveDevice{lveWindow};
        LVERenderer lveRenderer{lveWindow, lveDevice};

        std::vector<LVEGameObject> gameObjects;
    };
}  // namespace lve
