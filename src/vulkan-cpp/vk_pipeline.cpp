#include <vulkan-cpp/vk_pipeline.hpp>
#include <vulkan-cpp/vulkan-imports.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {
    vk_pipeline::vk_pipeline(GLFWwindow* p_window, const VkRenderPass& p_renderpass, const std::array<VkShaderModule, 2>& p_shader_modules) {
        int width=0;
        int height=0;
        m_driver = vk_driver::driver_context();

        glfwGetFramebufferSize(p_window, &width, &height);

        VkPipelineShaderStageCreateInfo vertex_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = p_shader_modules[0],
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragment_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = p_shader_modules[1],
            .pName = "main"
        };

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = { vertex_pipeine_stage_ci, fragment_pipeine_stage_ci};


        VkPipelineVertexInputStateCreateInfo vertex_input_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr, // Optional
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr, // Optional
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };
        // VkExtent2D swapchain_extent = vk::vk_swapchain::get_extent();
        //! TODO: Add this in later
        // VkExtent2D swapchain_extent = {};
        // VkExtent2D swapchain_extent = p_swapchain_extent_size;
        console_log_trace("W = {}, H = {}", width, height);
        // console_log_error("swapchain_extent needs a value because it has been removed!!!\nThis is a reminder if your using this function and calling!!!");
        // console_log_trace("W = {}", swapchain_extent.width);
        // console_log_trace("H = {}", swapchain_extent.height);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) width,
            .height = (float) height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = {(uint32_t)width, (uint32_t)height}
        };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        //! @note Rasterization
        VkPipelineRasterizationStateCreateInfo rasterizer_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE, // set to true make fragmenta that are beyond near/far planes clamped to them as opposed to discarding them
            .polygonMode = VK_POLYGON_MODE_FILL, // if set to true then geometry never passes through rasterizer stage. This basically disables output to frame_buffer
            .lineWidth = 1.0f,                  // represents thickness of lines
            .cullMode = VK_CULL_MODE_BACK_BIT,  // determines what culling to use. Can also be disabled, culls front-face, back-face or both
            .frontFace = VK_FRONT_FACE_CLOCKWISE, // specifies vertex order of fdaces considered front-face or clockwise/counter-clockwise
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f, // Optional
            .depthBiasSlopeFactor = 0.0f, // Optional
        };

        //! @note Multi-sampling
        VkPipelineMultisampleStateCreateInfo  multisampling_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f, // Optional
            .pSampleMask = nullptr, // Optional
            .alphaToCoverageEnable = VK_FALSE, // Optional
            .alphaToOneEnable = VK_FALSE, // Optional
        };

        // Depth Blending (will add later)

        // Color blending Attachment -- blending color when the fragment returns the color
        VkPipelineColorBlendAttachmentState color_blend_attachment = {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
            .colorBlendOp = VK_BLEND_OP_ADD, // Optional
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
            .alphaBlendOp = VK_BLEND_OP_ADD, // Optional
        };

        VkPipelineColorBlendStateCreateInfo color_blending_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
            .blendConstants[0] = 0.0f, // Optional
            .blendConstants[1] = 0.0f, // Optional
            .blendConstants[2] = 0.0f, // Optional
            .blendConstants[3] = 0.0f, // Optional
        };

        //! @note Dynamic State
        //! @note -- pipeline states needs to be baked into the pipeline state
        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0, // Optional
            .pSetLayouts = nullptr, // Optional
            .pushConstantRangeCount = 0, // Optional
            .pPushConstantRanges = nullptr, // Optional
        };

        vk::vk_check(vkCreatePipelineLayout(m_driver, &pipeline_layout_ci, nullptr, &m_pipeline_layout), "vkCreatePipelineLayout", __FUNCTION__);

        VkGraphicsPipelineCreateInfo graphics_pipeline_ci = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(shader_stages.size()),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer_ci,
            .pMultisampleState = &multisampling_ci,
            .pDepthStencilState = nullptr, // Optional
            .pColorBlendState = &color_blending_ci,
            .pDynamicState = &dynamic_state_ci,
            .layout = m_pipeline_layout,
            .renderPass = p_renderpass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        vk::vk_check(vkCreateGraphicsPipelines(m_driver, VK_NULL_HANDLE, 1, &graphics_pipeline_ci, nullptr, &m_pipeline), "vkCreateGraphicsPipelines", __FUNCTION__);
    }

    void vk_pipeline::destroy() {
        vkDestroyPipelineLayout(m_driver, m_pipeline_layout, nullptr);
        vkDestroyPipeline(m_driver, m_pipeline, nullptr);
    }

    void vk_pipeline::bind(const VkCommandBuffer& p_command_buffer) {}
};