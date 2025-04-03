#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_command_buffer.hpp>
#include <vector>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_renderpass.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>

namespace vk {
    class vk_imgui {
    public:
        vk_imgui();

        void initialize(const VkInstance& p_instance,
                        const VkPhysicalDevice& p_physical,
                        const vk_swapchain& p_swapchain);

        void begin();
        void end();

        void destroy();

    private:
        vk_driver m_driver;
        VkDescriptorPool m_imgui_desc_pool = nullptr;
        vk_swapchain m_current_swapchain;
        // VkRenderPass m_viewport_renderpass=nullptr;

        std::vector<vk_command_buffer> m_imgui_command_buffers;
    };
};