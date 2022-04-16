#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL  // To enable GLM hash functionality
#include "lve_model.hpp"

#include <tiny_obj_loader.h>

#include <cassert>
#include <cstring>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

#include "lve_utils.hpp"

namespace std {
    template <>
    struct hash<lve::LVEModel::Vertex> {
        size_t operator()(lve::LVEModel::Vertex const &vertex) const {
            size_t seed = 0;  // Stores final hash value
            lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std

namespace lve {

    LVEModel::LVEModel(LVEDevice &lveDevice, const LVEModel::Builder &builder)
        : lveDevice(lveDevice) {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    LVEModel::~LVEModel() {
        vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
        if (hasIndexBuffer) {
            vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
            vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
        }
    }

    std::unique_ptr<LVEModel> LVEModel::createModelFromFile(LVEDevice &device,
                                                            const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_unique<LVEModel>(device, builder);
    }

    void LVEModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        // First we create the staging buffer:
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        // Host: CPU - Device: GPU
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT => Allocated memory accessable from host
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT => Keep the host and device memory consistent with
        // each other. Whenever we update memort on the host, that data is automatically flushed to
        // the device side.
        lveDevice.createBuffer(
            bufferSize,
            // The buffer will be used as a source location for a memory transport operation.
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);
        void *data;
        // Create a region of host memory mapped to device memory and sets data to the beginning of
        // the mapped memory range.
        vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        // Copy data to the host mapped memory region. Because of
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, the host memory will automatically be flushed to
        // update the device memory. If this bit was absent, we had to call
        // vkFlushMappedMemoryRanges.
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT => Buffer is used for holding vertex input data.
        // VK_BUFFER_USAGE_TRANSFER_DST_BIT => Buffer is used as a transfer destination.
        lveDevice.createBuffer(bufferSize,
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               vertexBuffer,
                               vertexBufferMemory);
        // Perform a copy operation to move the contents of the staging buffer to the vertex buffer.
        lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
        // Cleanup the staging buffer as it is not needed anymore.
        vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
    }

    void LVEModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);
        void *data;
        vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        lveDevice.createBuffer(bufferSize,
                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               indexBuffer,
                               indexBufferMemory);
        lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
    }

    void LVEModel::bind(VkCommandBuffer commandBuffer) {
        // We can add multiple bindings by adding additional elements to these arrays.
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        // Record to the command buffer to bind one vertex buffer starting at binding 0 with an
        // offset of 0.
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            // indexType should match the type of the indices vector
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void LVEModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
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
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);
        // Binding is still 0 because we are interleaving position and color together in the one
        // binding.
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }

    void LVEModel::Builder::loadModel(const std::string &filepath) {
        // Stores position, color, normal, and uv coordinates
        tinyobj::attrib_t attrib;
        // Stores index values for each face elements
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }
        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    auto colorIndex = 3 * index.vertex_index + 2;
                    // Check if colors are provided
                    if (colorIndex < attrib.colors.size()) {
                        vertex.color = {
                            attrib.colors[colorIndex - 0],
                            attrib.colors[colorIndex - 1],
                            attrib.colors[colorIndex - 2],
                        };
                    } else {
                        // Set default color
                        vertex.color = {1.f, 1.f, 1.f};
                    }
                }
                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                // If vertex is new, add it to the uniqueVertices map
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}  // namespace lve
