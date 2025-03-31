#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <GLFW/glfw3.h>
// #include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/vk_shader.hpp>

namespace vk {
    class vk_pipeline {
    public:
        vk_pipeline(const VkRenderPass& p_renderpass, const vk_shader& p_shader_src, const VkDescriptorSetLayout& p_descriptor_sets);

        void bind(const VkCommandBuffer& p_command_buffer);

        void destroy();

        VkPipelineLayout get_layout() const { return m_pipeline_layout; }

    private:
        VkDevice m_driver=nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;
    };
};