#include <vulkan-cpp/vk_pipeline.hpp>
#include <vulkan-cpp/vulkan-imports.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_window.hpp>

namespace vk {
    vk_pipeline::vk_pipeline(
      const VkRenderPass& p_renderpass,
      vk_shader& p_shader_src,
      const VkDescriptorSetLayout& p_descriptor_sets) {
        int width = 0;
        int height = 0;
        m_driver = vk_driver::driver_context();
        GLFWwindow* handle = vk_window::native_window();

        console_log_info("vk_pipeline begin initialization!!!");

        glfwGetFramebufferSize(handle, &width, &height);
        VkShaderModule vert_module = p_shader_src.get_vertex_module();
        VkShaderModule frag_module = p_shader_src.get_fragment_module();

        if (vert_module != nullptr) {
            console_log_trace("vertex shader module is valid!!!");
        }

        if (frag_module != nullptr) {
            console_log_trace("fragment shader module is valid!!!");
        }

        VkPipelineShaderStageCreateInfo vertex_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_module,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragment_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_module,
            .pName = "main"
        };

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {
            vertex_pipeine_stage_ci, fragment_pipeine_stage_ci
        };

        const std::span<VkVertexInputBindingDescription> bind_vertex_attributes = p_shader_src.get_vertex_bind_attributes();
        const std::span<VkVertexInputAttributeDescription> vertex_attributes = p_shader_src.get_vertex_attributes();

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(bind_vertex_attributes.size()),
            .pVertexBindingDescriptions = bind_vertex_attributes.data(), // Optional: Enabled
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size()),
            .pVertexAttributeDescriptions = vertex_attributes.data(), // Optional: Enabled
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType =
              VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };
        // VkExtent2D swapchain_extent = vk::vk_swapchain::get_extent();
        console_log_trace("W = {}, H = {}", width, height);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)width,
            .height = (float)height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = { .offset = { 0, 0 },
                             .extent = { (uint32_t)width, (uint32_t)height } };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        //! @note Rasterization
        VkPipelineRasterizationStateCreateInfo rasterizer_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable =
              VK_FALSE, // set to true make fragmenta that are beyond near/far
                        // planes clamped to them as opposed to discarding them
            .polygonMode =
              VK_POLYGON_MODE_FILL, // if set to true then geometry never passes
                                    // through rasterizer stage. This basically
                                    // disables output to frame_buffer
            .lineWidth = 1.0f,      // represents thickness of lines
            .cullMode =
              VK_CULL_MODE_BACK_BIT, // determines what culling to use. Can also
                                     // be disabled, culls front-face, back-face
                                     // or both
            .frontFace =
              VK_FRONT_FACE_CLOCKWISE, // specifies vertex order of fdaces
                                       // considered front-face or
                                       // clockwise/counter-clockwise
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f,          // Optional
            .depthBiasSlopeFactor = 0.0f,    // Optional
        };

        //! @note Multi-sampling
        VkPipelineMultisampleStateCreateInfo multisampling_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,          // Optional
            .pSampleMask = nullptr,            // Optional
            .alphaToCoverageEnable = VK_FALSE, // Optional
            .alphaToOneEnable = VK_FALSE,      // Optional
        };

        // Depth Blending (will add later)

        // Color blending Attachment -- blending color when the fragment returns
        // the color
        VkPipelineColorBlendAttachmentState color_blend_attachment = {
            .blendEnable = true,
            .srcColorBlendFactor =
              VK_BLEND_FACTOR_SRC_ALPHA, // Enabled: alpha blending
            .dstColorBlendFactor =
              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // Enabled: alpha blending
            .colorBlendOp = VK_BLEND_OP_ADD,       // Enabled: alpha blending
            .srcAlphaBlendFactor =
              VK_BLEND_FACTOR_ONE, // Enabled: alpha blending
            .dstAlphaBlendFactor =
              VK_BLEND_FACTOR_ZERO,          // Enabled: alpha blending
            .alphaBlendOp = VK_BLEND_OP_ADD, // Enabled: alpha blending
            .colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blending_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
            // these are optional
            .blendConstants = { 0.f, 0.f, 0.f, 0.f }
            // .blendConstants[0] = 0.0f, // Optional
            // .blendConstants[1] = 0.0f, // Optional
            // .blendConstants[2] = 0.0f, // Optional
            // .blendConstants[3] = 0.0f, // Optional
        };

        // Enable depth-stencil state
        VkPipelineDepthStencilStateCreateInfo pipeline_deth_stencil_state_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
        };

        //! @note Dynamic State
        //! @note -- pipeline states needs to be baked into the pipeline state
        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        VkDescriptorSetLayout layout = p_descriptor_sets;

        vk::vk_check(
          vkCreatePipelineLayout(
            m_driver, &pipeline_layout_ci, nullptr, &m_pipeline_layout),
          "vkCreatePipelineLayout",
          __FUNCTION__);

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
            // .pDepthStencilState = nullptr, // Optional
            .pDepthStencilState = &pipeline_deth_stencil_state_ci,
            .pColorBlendState = &color_blending_ci,
            .pDynamicState = &dynamic_state_ci,
            .layout = m_pipeline_layout,
            .renderPass = p_renderpass,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        vk::vk_check(
          vkCreateGraphicsPipelines(
            m_driver, nullptr, 1, &graphics_pipeline_ci, nullptr, &m_pipeline),
          "vkCreateGraphicsPipelines",
          __FUNCTION__);

        console_log_info("vk_pipeline successfully initialized!!!!\n\n");
    }

    void vk_pipeline::destroy() {
        vkDestroyPipelineLayout(m_driver, m_pipeline_layout, nullptr);
        vkDestroyPipeline(m_driver, m_pipeline, nullptr);
    }

    void vk_pipeline::bind(const VkCommandBuffer& p_command_buffer) {
        vkCmdBindPipeline(
          p_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }
};