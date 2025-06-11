#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_command_buffer.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_buffer.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/vk_renderpass.hpp>

namespace vk {
    class vk_imgui {
    public:
        vk_imgui();

        void initialize(const VkInstance& p_instance,
                        const VkPhysicalDevice& p_physical, const VkRenderPass& p_rp, uint32_t p_image_size, const VkSurfaceFormatKHR& p_surface_format);

        void begin();
        void end(const VkCommandBuffer& p_current);

        void destroy();


    private:
        void create_imgui_renderpass();
        void create_imgui_framebuffers();
        void create_viewport_images();
        void create_viewport_imageview();

    private:
        vk_driver m_driver;
        VkDescriptorPool m_imgui_desc_pool = nullptr;
        VkCommandPool m_imgui_command_pool=nullptr;

        std::vector<vk_image> m_viewport_images;
        std::vector<VkFramebuffer> m_viewport_framebuffers;

        vk_renderpass m_imgui_renderpass;
        // vk_swapchain m_current_swapchain;
        // VkRenderPass m_viewport_renderpass=nullptr;

        // std::vector<VkImage> m_viewport_images;
        // std::vector<VkImageView>
    };
};
