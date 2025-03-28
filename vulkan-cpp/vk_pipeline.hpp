#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <GLFW/glfw3.h>
#include <vulkan-cpp/vk_descriptor_set.hpp>

namespace vk {
    class vk_pipeline {
    public:
        vk_pipeline(GLFWwindow* p_window, const VkRenderPass& p_renderpass, const VkShaderModule& p_vert_module, const VkShaderModule& p_frag_module, const vk_descriptor_set& p_descriptor_sets, const vk_vertex_buffer& p_vertex_buffer, const std::vector<buffer_properties>& p_buffer_data, uint32_t p_size_in_bytes);

        void bind(const VkCommandBuffer& p_command_buffer);

        void destroy();

        VkPipelineLayout get_layout() const { return m_pipeline_layout; }

    private:
        VkDevice m_driver=nullptr;
        vk_descriptor_set m_descriptor_set;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;
    };
};