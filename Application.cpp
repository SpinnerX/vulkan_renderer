#include <vulkan/vulkan_core.h>
#include <fmt/core.h>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/vk_renderpass.hpp>

static std::vector<char> readFile(const std::string& p_filename){
    std::ifstream ins(p_filename, std::ios::ate | std::ios::binary);
    if(!ins){
        return {};
    }
    else{
        console_log_trace("Successfully Read Shader File = {}", p_filename);
    }

    size_t size = (size_t)ins.tellg();

    std::vector<char> buffer(size);
    ins.seekg(0);
    ins.read(buffer.data(), size);
    ins.close();

    return buffer;
}

VkShaderModule CreateShaderModule(const VkDevice& p_driver, const std::vector<char>& p_code) {
    VkShaderModuleCreateInfo module_ci = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = p_code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(p_code.data())
    };


    VkShaderModule shader_module;
    vk::vk_check(vkCreateShaderModule(p_driver, &module_ci, nullptr, &shader_module), "vkCreateShaderModule", __FUNCTION__);

    return shader_module;
}

class shader_tutorial {
public:

    shader_tutorial(const VkDevice& p_driver) : m_driver(p_driver){
        create_shader_modules();
    }

    ~shader_tutorial() {
        // vkDestroyRenderPass(m_driver, m_shader_renderpass, nullptr);
        // vkDestroyPipeline(m_driver, m_graphics_pipeline, nullptr);
        // vkDestroyPipelineLayout(m_driver, m_graphics_pipeline_layout, nullptr);

        // vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);
        // vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
        cleanup();
    }

    void cleanup(){
        vkDestroyPipeline(m_driver, m_graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(m_driver, m_graphics_pipeline_layout, nullptr);

        vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);
        vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
    }


    void create_shader_modules() {
        // First we read the shader
        auto vertex_shader = readFile("shaders/vert.spv");
        auto fragment_shader = readFile("shaders/frag.spv");

        // Then we setup the shader module
        m_vertex_shader_module = CreateShaderModule(m_driver, vertex_shader);
        m_fragment_shader_module = CreateShaderModule(m_driver, fragment_shader);
    }


    void create_pipeline_shader_stages(const VkRenderPass& p_renderpass) {
        VkPipelineShaderStageCreateInfo vertex_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = m_vertex_shader_module,
            .pName = "main"
        };

        VkPipelineShaderStageCreateInfo fragment_pipeine_stage_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = m_fragment_shader_module,
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
        VkExtent2D swapchain_extent = vk::vk_swapchain::get_extent();
        console_log_trace("W = {}", swapchain_extent.width);
        console_log_trace("H = {}", swapchain_extent.height);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) swapchain_extent.width,
            .height = (float) swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapchain_extent,
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

        vk::vk_check(vkCreatePipelineLayout(m_driver, &pipeline_layout_ci, nullptr, &m_graphics_pipeline_layout), "vkCreatePipelineLayout", __FUNCTION__);

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
            .layout = m_graphics_pipeline_layout,
            .renderPass = p_renderpass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        vk::vk_check(vkCreateGraphicsPipelines(m_driver, VK_NULL_HANDLE, 1, &graphics_pipeline_ci, nullptr, &m_graphics_pipeline), "vkCreateGraphicsPipelines", __FUNCTION__);

        // vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);
        // vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
    }

    VkPipeline get_graphics_pipeline() const { return m_graphics_pipeline; }

private:
    VkDevice m_driver=nullptr;
    VkShaderModule m_vertex_shader_module;
    VkShaderModule m_fragment_shader_module;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
};

VkCommandPool create_command_pool(const VkDevice& p_driver, uint32_t p_graphics_index) {
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo cmd_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = p_graphics_index,
    };

    vk::vk_check(vkCreateCommandPool(p_driver, &cmd_pool_create_info, nullptr, &command_pool), "vkCreateCommandPool", __FUNCTION__);

    return command_pool;
}

VkCommandBuffer create_command_buffers(const VkDevice& p_driver, const VkCommandPool& p_command_pool) {
    // VkCommandPool command_pool = create_command_pool(p_driver, p_graphics_index);
    VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = p_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    vk::vk_check(vkAllocateCommandBuffers(p_driver, &cmd_buffer_alloc_info, &command_buffer), "vkAllocateCommandBuffers", __FUNCTION__);
    return command_buffer;
}

std::vector<VkCommandBuffer> create_command_buffers_vec(const VkDevice& p_driver, const VkCommandPool& p_command_pool) {
    // VkCommandPool command_pool = create_command_pool(p_driver, p_graphics_index);
    VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = p_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    std::vector<VkCommandBuffer> command_buffer(vk::swapchain_configs::MaxFramesInFlight);
    vk::vk_check(vkAllocateCommandBuffers(p_driver, &cmd_buffer_alloc_info, command_buffer.data()), "vkAllocateCommandBuffers", __FUNCTION__);
    return command_buffer;
}

/**
 * @name p_current_command_buffer
 * @note The current command buffer to record/write data to
 * 
 * @name p_current_renderpass
 * @note The renderpass to record the current command buffer too
*/
template<typename UFunction>
void record_commands(const VkCommandBuffer& p_current_command_buffer, const UFunction& p_callable) {

    // Begin command buffer recording operation
    VkCommandBufferBeginInfo cmd_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,// Optional
        .pInheritanceInfo = nullptr, // Optional
    };

    vk::vk_check(vkBeginCommandBuffer(p_current_command_buffer, &cmd_buffer_begin_info), "vkBeginCommandBuffer", __FUNCTION__);

    p_callable(p_current_command_buffer);

    vk::vk_check(vkEndCommandBuffer(p_current_command_buffer), "vkEndCommandBuffer", __FUNCTION__);

}

int main(){
    logger::console_log_manager::initialize_logger_manager();

    //! @note Initializing GLFW
    if(!glfwInit()){
        fmt::println("glfwInit Initialized!!!");
        return -1;
    }

    //! @note Setup GLFW Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int width = 800;
    int height = 600;

    //! @note 0.) Initialize Vulkan
    // create_vulkan_instance();
    vk::vk_context initiating_vulkan = vk::vk_context("vulkan");
    
    //! @note 1.) Initialize GLFW Window
    vk::vk_window main_window = vk::vk_window("Vulkan Window", width, height);

    //! @note 2.) Initiates Vulkan Surface
    main_window.create_window_surface(initiating_vulkan);
    main_window.center_window();

    //! @note 3.) Initialize Vulkan physical and logical drivers
    vk::vk_physical_driver main_physical_device = vk::vk_physical_driver(initiating_vulkan);
    vk::vk_driver main_driver = vk::vk_driver(main_physical_device);

    //! @note 4.) Initializing Swapchain
    vk::vk_swapchain main_window_swapchain = vk::vk_swapchain(main_physical_device, main_driver, main_window);

    // shader_tutorial first_shader = shader_tutorial(main_driver);
    
    // //! @note 5.) Creating a render pass for the graphics pipeline
    // // vk::vk_renderpass main_swapchain_renderpass = vk::vk_renderpass(main_driver, main_window_swapchain.get_format());
    // // first_shader.create_pipeline_shader_stages(main_swapchain_renderpass);
    // first_shader.create_pipeline_shader_stages(main_window_swapchain.get_renderpass());

    // VkPipeline graphics_pipeline = first_shader.get_graphics_pipeline();

    // //! @note 6.) Create Command Pools, Buffers
    // VkCommandPool command_pool = create_command_pool(main_driver, main_physical_device.get_queue_indices().Graphics);
    // // VkCommandBuffer command_buffer = create_command_buffers(main_driver, command_pool);
    // std::array<VkCommandBuffer, vk::swapchain_configs::MaxFramesInFlight> swapchain_command_buffers;

    // for(size_t i = 0; i < swapchain_command_buffers.size(); i++) {
    //     swapchain_command_buffers[i] = create_command_buffers(main_driver, command_pool);
    // }

    // uint32_t frame_index = 0;

    while(main_window.is_active()){

        // acquire next image

        // draw

        // presenting frame
        
        glfwPollEvents();
    }

    main_window_swapchain.clean();
    main_driver.clean();

}