#include "lve_pipeline.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve {

    LVEPipeline::LVEPipeline(LVEDevice& device,
                             const std::string& vertFilepath,
                             const std::string& fragFilepath,
                             const PipelineConfigInfo& configInfo)
        : lveDevice(device) {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    LVEPipeline::~LVEPipeline() {
        vkDestroyShaderModule(lveDevice.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> LVEPipeline::readFile(const std::string& filepath) {
        // std::ios::ate => Immediately seek the end of file.
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        // Seek to the start of the file
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void LVEPipeline::createGraphicsPipeline(const std::string& vertFilepath,
                                             const std::string& fragFilepath,
                                             const PipelineConfigInfo& configInfo) {
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
               "Cannor create graphics pipeline:: no pipelineLayout provided in configInfo");
        assert(configInfo.renderPass != VK_NULL_HANDLE &&
               "Cannor create graphics pipeline:: no renderPass provided in configInfo");

        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        // std::cout << "Vertex shader code size: " << vertCode.size() << "\n";
        // std::cout << "Fragment shader code size: " << fragCode.size() << "\n";

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        // Name of entry function in vertex shader
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        // Describe how to interpret vertex buffer data that is the initial input into the graphics
        // pipeline.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // Zero for now because we have hard coded vertex data directly into the shader.
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // The numver of programmable stages our pipeline will use
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        // Optional: Will use this later.
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
                lveDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
    }

    void LVEPipeline::createShaderModule(const std::vector<char>& code,
                                         VkShaderModule* shaderModule) {
        // Configure this struct and pass to the function
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module.");
        }
    }

    PipelineConfigInfo LVEPipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipelineConfigInfo configInfo{};
        // sType is the member that defines the struct type.

        // First stage of pipeline: Input Assembler
        // Inputs list of vertices and groups them into geometry
        configInfo.inputAssemblyInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // How does the input assembler know that we want each of the three vertices to be a
        // triangle?
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: Every three vertices are grouped together into a
        // separate triangle.
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: Every additional vertex uses the previous two
        // vertices to form the next triangle. More triangles with less memory. Limits the geometry
        // to be a connected strip of triangles. => To break this limit, we can use the
        // primitiveRestartEnable. By setting primitiveRestartEnable to true, we can break up a
        // strip by inserting a special value into an index buffer.
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Viewport: Describes the transformation between the pipeline's output and the target
        // image.
        /* Viewport Transformation
                          x                                            x
             ---------------------------->                ---------------------------->
         (-1,-1)                        (1,-1)         (0,0)
            +-----------------------------+              +-----------------------------+
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
        y | |                             |  ------> y | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          | |                             |            | |                             |
          v |                             |            v |                             |
            +-----------------------------+              +-----------------------------+
         (-1,1)                         (1,1)                                    (width,height)
         */
        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        // For example, if we set the height to height*0.5f, the image would squashed into the top
        // half.
        configInfo.viewport.height = static_cast<float>(height);
        // Depth range for the viewport
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;

        // Scissor
        // Like viewport, but instead of squashing the triangle, it cuts it.
        // For example, if we set the height to height*0.5f, the bottom half of the image would be
        // cut.
        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};

        // Combine viewport and scissor into a single VIEWPORT_STATE_CREATE_INFO
        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = &configInfo.viewport;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = &configInfo.scissor;

        // Rasterization: This stage breaks up our geomerty into fragments for each pixel our
        // triangle overlaps.
        configInfo.rasterizationInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // If true, values less than zero are clamped to zero, and values greater than one are
        // clamped to one.
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        // rasterizerDiscardEnable discards all primitives before rasterization.
        // Only used when we only want to use the first stages of the graphics pipeline.
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        // When drawing triangles, do we want to draw just the corners, the edges, or the triangle
        // filled in?
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        // We can optionally discard triangles based on their apparent facing or winding order.
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        // Which direction do we want to use as the front face? Clockwise or counter clockwise.
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        // We can alter depth values by adding a constant value or by a factor of the fragment's
        // slope.
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        // MSAA: Multisample Anti-aliasing.
        // Multiple samples are taken along the edges of the geometry.
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        // Color Blending: Controls how we combine colors in the frame buffer.
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        // Depth buffer
        configInfo.depthStencilInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        // No default for pipelineLayout, renderPass, and subpass.

        return configInfo;
    }

}  // namespace lve