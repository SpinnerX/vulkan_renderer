#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <GLFW/glfw3.h>

namespace vk {
    class vk_pipeline {
    public:
        vk_pipeline(GLFWwindow* p_window, const VkRenderPass& p_renderpass, const std::array<VkShaderModule, 2>& p_shader_modules);

        void bind(const VkCommandBuffer& p_command_buffer);

        void destroy();

    private:
        VkDevice m_driver=nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;
    };
};