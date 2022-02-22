#include "lve_model.hpp"

#include <cassert>
#include <cstring>

namespace lve {

    LVEModel::LVEModel(LVEDevice &lveDevice, const std::vector<Vertex> &vertices)
        : lveDevice(lveDevice) {
        createVertexBuffers(vertices);
    }

    LVEModel::~LVEModel() {
        vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
    }

    void LVEModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        // Host: CPU - Device: GPU
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT => Allocated memory accessable from host
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT => Keep the host and device memory consistent with
        // each other
        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,  // Buffer is used for holding vertex input data
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory);
        void *data;
        // Create a region of host memory mapped to device memory and sets data to the beginning of
        // the mapped memory range.
        vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        // Copy data to the host mapped memory region. Because of
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, the host memory will automatically be flushed to
        // update the device memory. If this bit was absent, we had to call
        // vkFlushMappedMemoryRanges.
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
    }

    void LVEModel::bind(VkCommandBuffer commandBuffer) {
        // We can add multiple bindings by adding additional elements to these arrays.
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        // Record to the command buffer to bind one vertex buffer starting at binding 0 with an
        // offset of 0.
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void LVEModel::draw(VkCommandBuffer commandBuffer) {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    std::vector<VkVertexInputBindingDescription> LVEModel::Vertex::getBindingDescriptions() {
        // This is for our single vertex buffer.
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        // We use sizeof(Vertex), so we don't have to change this if we change the Vertex struct.
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LVEModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);
        // Binding is still 0 because we are interleaving position and color together in the one
        // binding.
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }

}  // namespace lve
