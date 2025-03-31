#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <GLFW/glfw3.h>
// #include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/vk_shader.hpp>
#include <span>

namespace vk {

    // Uses VertexInputAttributeDescription
    // typedef struct VkPipelineVertexInputStateCreateInfo {
    //     VkStructureType                             sType;
    //     const void*                                 pNext;
    //     VkPipelineVertexInputStateCreateFlags       flags;
    //     uint32_t                                    vertexBindingDescriptionCount;
    //     const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
    //     uint32_t                                    vertexAttributeDescriptionCount;
    //     const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
    // } VkPipelineVertexInputStateCreateInfo;

    // VkVertexInputAttributeDescription
    // typedef struct VkVertexInputAttributeDescription {
    //     uint32_t    location;
    //     uint32_t    binding;
        // VkFormat    format;
    //     uint32_t    offset;
    // } VkVertexInputAttributeDescription;



    /*
    [ These are my thoughts ]

    Consider the following shader below

    layout(binding = 0) uniform UniformBufferObject {
        mat4 model;
        mat4 view;
        mat4 proj;
    } ubo;

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inColor;
    layout(location = 2) in vec2 inTexCoord;

    layout(location = 0) out vec3 fragColor;
    layout(location = 1) out vec2 fragTexCoord;
    

    Vulkan Descriptor Set Layout Bindings
        - Probably have layout bindings and per descriptor sets would provide them to the pipeline
        - Because pipeline can be reused by descriptor sets, we'd essentially just pass in our vertex_attribute_layout struct to both of these
        - Each vk_pipeline abstraction may take in std::span<vertex_attribute_layout> that will be passed into both descriptor set layout and VkPipeline
        - Another approach is we'd have two different types of pipeline layouts as well.


    Vulkan Pipeline Input Vertex Attributes
    
    
    
    */




    struct pipeline_vertex_attributes {
        std::string Name="Undefined";
        uint32_t Binding = 0;
        uint32_t Location=0;
        VkFormat Format=VkFormat::VK_FORMAT_UNDEFINED;
        uint32_t Offset=0;
    };
    
    class vk_pipeline {
    public:
        vk_pipeline(const VkRenderPass& p_renderpass, const vk_shader& p_shader_src, const VkDescriptorSetLayout& p_descriptor_sets, const std::span<pipeline_vertex_attributes>& p_vertex_attributes);

        void bind(const VkCommandBuffer& p_command_buffer);

        void destroy();

        VkPipelineLayout get_layout() const { return m_pipeline_layout; }

    private:
        VkDevice m_driver=nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;
    };
};