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
    //     uint32_t vertexBindingDescriptionCount; const
    //     VkVertexInputBindingDescription*      pVertexBindingDescriptions;
    //     uint32_t vertexAttributeDescriptionCount; const
    //     VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
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
        - Probably have layout bindings and per descriptor sets would provide
    them to the pipeline
        - Because pipeline can be reused by descriptor sets, we'd essentially
    just pass in our vertex_attribute_layout struct to both of these
        - Each vk_pipeline abstraction may take in
    std::span<vertex_attribute_layout> that will be passed into both descriptor
    set layout and VkPipeline
        - Another approach is we'd have two different types of pipeline layouts
    as well.


    Vulkan Pipeline Input Vertex Attributes



    */

    /*

        Usage of vertex_binding_description
        { "Vertex", .Binding = 0, .sttride = sizeof(vertex), .InputRate =
       VK_VERTEX_INPUT_RATE_VERTEX}
    */
    struct vertex_binding_description {
        vertex_binding_description(const std::string& p_name,
                                   uint32_t p_binding,
                                   uint32_t p_stride,
                                   const VkVertexInputRate& p_input_rate)
          : Name(p_name)
          , Binding(p_binding)
          , Stride(p_stride)
          , InputRate(p_input_rate) {}
        std::string Name = "Undefined";
        uint32_t Binding = 0;
        uint32_t Stride = 0;
        VkVertexInputRate InputRate;
    };

    /*

        Usage
        { "inPosition", location = 0, binding = 0, .offset =
       offsetof(vk::vertex, Position)} { "inColor", location = 1, binding = 0,
       .offset = offsetof(vk::vertex, Color) } { "inColor", location = 2,
       binding = 0, .offset = offsetof(vk::vertex, inTexCoords) }

        Used for shaders that do something similar or the following vertex
       attributes definition layout(location = 0) in vec3 inPosition;
        layout(location = 1) in vec3 inColor;
        layout(location = 2) in vec2 inTexCoord;
    */
    struct pipeline_vertex_attributes {
        pipeline_vertex_attributes(const std::string p_name,
                                   uint32_t p_binding,
                                   uint32_t p_location,
                                   uint32_t p_offset,
                                   const VkFormat& p_format)
          : Name(p_name)
          , Binding(p_binding)
          , Location(p_location)
          , Format(p_format) {}
        std::string Name = "Undefined";
        uint32_t Binding = 0;
        uint32_t Location = 0;
        uint32_t Offset = 0;
        VkFormat Format = VkFormat::VK_FORMAT_UNDEFINED;
    };

    class vk_pipeline {
    public:
        vk_pipeline(
          const VkRenderPass& p_renderpass,
          vk_shader& p_shader_src,
          const VkDescriptorSetLayout& p_descriptor_sets);

        void bind(const VkCommandBuffer& p_command_buffer);

        void destroy();

        VkPipelineLayout get_layout() const { return m_pipeline_layout; }

    private:
        VkDevice m_driver = nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;
    };
};