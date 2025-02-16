#include "vulkan-cpp/vulkan_driver.hpp"
#include "vulkan-cpp/vulkan_physical_driver.hpp"
#include "vulkan-cpp/vulkan_context.hpp"
#include "vulkan-cpp/vulkan_swapchain.hpp"

#include "vulkan-cpp/vulkan-imports.hpp"
#include <fstream>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include "vulkan-cpp/helper_functions.hpp"

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule createShaderModule(const VkDevice& p_device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(p_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

namespace vk{
class vulkan_shader{
public:
    vulkan_shader(const VkDevice& p_device, const std::string& p_vertex, const std::string& p_fragment){
        fmt::println("\nVulkan Shader Begin Initialization!!!");
        auto vertex_shader = readFile(p_vertex);
        auto fragment_shader = readFile(p_fragment);

        VkShaderModule vert_shader_module = createShaderModule(p_device, vertex_shader);
        VkShaderModule frag_shader_module = createShaderModule(p_device, fragment_shader);
        
        VkPipelineShaderStageCreateInfo vert_shader_stage_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader_module,
            .pName = "main",
        };

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader_module,
            .pName = "main",
        };

        std::array<VkPipelineShaderStageCreateInfo, 2> stages = {vert_shader_stage_info, frag_shader_stage_info};
        
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        fmt::println("Vulkan Shader End Initialization!!!\n");


        fmt::println("\nVulkan Begin Graphics Pipeline Layout Initialization!!");

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            // pipelineLayoutInfo.setLayoutCount = 1,
            // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout,
        };

        vk_check(vkCreatePipelineLayout(p_device, &pipelineLayoutInfo, nullptr, &m_graphics_pipeline_layout), "vkCreateGraphicsPipelines", __FILE__, __LINE__, __FUNCTION__);

        fmt::println("\nVulkan End Graphics Pipeline Layout Initialization Successfullyy!!!");
    }

private:
    VkPipelineLayout m_graphics_pipeline_layout = nullptr;
};

}; // end of vk namespace

int main(){

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
    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan Window", nullptr, nullptr);
    
    //! @note 1.) Initialize Vulkan
    vk::context create_context = vk::context();

    //! @note 2.) Creating window surface (requires vulkan to be instantiated)
    VkSurfaceKHR surface = nullptr;
    VkResult surface_result = glfwCreateWindowSurface(vk::context::get_instance(), window, nullptr, &surface);

    if(surface_result != VK_SUCCESS){
        fmt::println("Surface VkResult Failed with status === {}", (int)surface_result);
        return -1;
    }
    glfwMakeContextCurrent(window);


    //! @note 4.) Initiates vulkan physical and logical drivers
    vk::physical_driver physical = vk::physical_driver(create_context);
    vk::driver logical_driver = vk::driver(physical);


    //! @note 4.) Initializing Swapchain
    // create_swapchain(physical, logical_driver);
    vk::swapchain main_swapchain = vk::swapchain(physical, logical_driver, surface);
    main_swapchain.on_create(width, height);

    vk::vulkan_shader shader_and_graphics_pipeline = vk::vulkan_shader(logical_driver, "shaders/vert.spv", "shaders/frag.spv");


    while(!glfwWindowShouldClose(window)){

        glfwPollEvents(); 
    }
}