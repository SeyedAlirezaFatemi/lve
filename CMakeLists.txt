cmake_minimum_required (VERSION 3.8)
project ("vulkan-engine-cmake")

# For VS Code. Use -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake otherwise!
include(C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory("vulkan-engine")
