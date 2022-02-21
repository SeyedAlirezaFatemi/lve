#pragma once

// Signal GLM to expect angles to be specified in radians
#define GLM_FORCE_RADIANS
// Signal GLM to expect the depth buffer values to range from 0 to 1. OpenGL is -1 to 1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

#include "lve_device.hpp"

namespace lve {
    class LVEModel {
        /**
         * @brief The purpose of this class is to take vertex data created by or read from a file by
         * the CPU, allocate the memory, and copy the data over to the GPU.
         */
       public:
        struct Vertex {
            glm::vec2 position;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        LVEModel(LVEDevice &lveDevice, const std::vector<Vertex> &vertices);
        ~LVEModel();

        // Delete the copy constructor and operator.
        // That's because this class manages the buffer and memory objects.
        LVEModel(const LVEModel &) = delete;
        LVEModel &operator=(const LVEModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

       private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        LVEDevice &lveDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}  // namespace lve
