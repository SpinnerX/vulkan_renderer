#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <GLFW/glfw3.h>
#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/vk_shader.hpp>

namespace vk {
    class vk_pipeline {
    public:
        vk_pipeline(GLFWwindow* p_window, const VkRenderPass& p_renderpass, const vk_shader& p_shader_src, const vk_descriptor_set& p_descriptor_sets);

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