cmake_minimum_required (VERSION 3.8)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(vulkan REQUIRED)

add_executable (vulkan-engine  "main.cpp" "first_app.cpp" "first_app.hpp"
                "lve_window.cpp" "lve_window.hpp" "lve_pipeline.hpp" "lve_pipeline.cpp"
                "lve_device.hpp" "lve_device.cpp" "lve_swap_chain.hpp" "lve_swap_chain.cpp"
                "lve_model.hpp" "lve_model.cpp" "lve_game_object.hpp"
                "lve_renderer.hpp" "lve_renderer.cpp"
                "simple_render_system.hpp" "simple_render_system.cpp"
                "lve_camera.hpp" "lve_camera.cpp"
                "keyboard_movement_controller.hpp" "keyboard_movement_controller.cpp")

target_link_libraries(vulkan-engine PRIVATE glm::glm)
target_link_libraries(vulkan-engine PRIVATE glfw)
target_link_libraries(vulkan-engine PRIVATE Vulkan::Vulkan)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../libs/tinyobjloader/)
