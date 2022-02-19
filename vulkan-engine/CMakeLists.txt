cmake_minimum_required (VERSION 3.8)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(vulkan REQUIRED)

add_executable (vulkan-engine  "main.cpp" "first_app.cpp" "first_app.hpp"
                "lve_window.cpp" "lve_window.hpp" "lve_pipeline.hpp" "lve_pipeline.cpp"
                "lve_device.hpp" "lve_device.cpp")

target_link_libraries(vulkan-engine PRIVATE glm::glm)
target_link_libraries(vulkan-engine PRIVATE glfw)
target_link_libraries(vulkan-engine PRIVATE Vulkan::Vulkan)
