#include "simple_render_system.hpp"

// Signal GLM to expect angles to be specified in radians
#define GLM_FORCE_RADIANS
// Signal GLM to expect the depth buffer values to range from 0 to 1. OpenGL is -1 to 1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <array>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>

namespace lve {

    struct SimplePushConstantData {
        glm::mat4 transform{1.f};     // Default initialize to identity
        alignas(16) glm::vec3 color;  // Pay attention to alignment requirements
    };

    SimpleRenderSystem::SimpleRenderSystem(LVEDevice& device, VkRenderPass renderPass)
        : lveDevice{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        // We want access to the push constant data in both the vertex and fragment shaders.
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // A pipeline set layout is used to pass data other than vertex data to the vertex and
        // fragment shaders. Like textures.
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        // Push constants are a way to efficiently send a small amount of data to the shader
        // programs.
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(
                lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LVEPipeline::defaultPipelineConfigInfo(pipelineConfig);
        // A render pass describes the structure and format of frame buffer objects and their
        // attachments.
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LVEPipeline>(
            lveDevice,
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.vert.spv",
            "..\\..\\..\\..\\vulkan-engine\\shaders\\simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
                                               std::vector<LVEGameObject>& gameObjects) {
        // Render
        lvePipeline->bind(commandBuffer);
        for (auto& obj : gameObjects) {
            obj.transform.rotation.y =
                glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
            obj.transform.rotation.x =
                glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.color = obj.color;
            push.transform = obj.transform.mat4();

            vkCmdPushConstants(commandBuffer,
                               pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}  // namespace lve
