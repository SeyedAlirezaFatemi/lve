#pragma once
#include <string>
#include <vector>

#include "lve_device.hpp"

namespace lve {
    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class LVEPipeline {
       public:
        LVEPipeline(LVEDevice& device,
                    const std::string& vertFilepath,
                    const std::string& fragFilepath,
                    const PipelineConfigInfo& configInfo);

        ~LVEPipeline();

        // delete copy constructor
        LVEPipeline(const LVEPipeline&) = delete;
        // delete assignment operator
        LVEPipeline& operator=(const LVEPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

       private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeline(const std::string& vertFilepath,
                                    const std::string& fragFilepath,
                                    const PipelineConfigInfo& configInfo);

        // VkShaderModule* is pointer to pointer.
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        // Memory unsafe in general. The device will outlive any instances of the class here.
        LVEDevice& lveDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;  // This is a pointer. Hover over it to see!
        VkShaderModule fragShaderModule;  // This is a pointer. Hover over it to see!
    };
}  // namespace lve
