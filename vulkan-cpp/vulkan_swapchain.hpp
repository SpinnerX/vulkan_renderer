#pragma once
#include "vulkan_physical_driver.hpp"
#include "vulkan_driver.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vk{
    class swapchain{
    public:
        //! TODO: Probably want to have something like: vk::context::get_current_physical_driver();
        //! TODO: Also want to additionally add vk::context::get_current_driver() as well
        //! @note These get called at construction rather then explicitly calling static everywhere
        swapchain(const physical_driver& p_physical, const driver& p_driver, const VkSurfaceKHR& p_surface);

        void on_create(uint32_t p_width, uint32_t p_height);

        VkExtent2D get_extent() const { return m_swapchain_extent; }


    private:
        void on_swapchain_format_query(const VkPhysicalDevice& p_physical);
        void on_swapchain_presentation_query();
        void on_create_images();

        void on_create_swapchain_renderpass();

        void on_create_swapchain_framebuffers();


    private:
        physical_driver m_physical;
        driver m_driver;

        // swapchain details
        struct swapchain_image{
            VkImage Image; // write-only image
            VkImageView ImageView; // read-only image

            bool DepthEnabled=false; // checks if depth are enabled for specific images
            VkDeviceMemory DeviceMemory;
        };

        std::vector<swapchain_image> m_swapchain_images;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // vulkan swapchain-specific details
        // SwapChainSupportDetails
        VkSurfaceCapabilitiesKHR m_surface_capabilities;
        VkSurfaceFormatKHR m_current_selected_surface_format;
        VkPresentModeKHR m_presentation_mode;
        VkExtent2D m_swapchain_extent;

        VkSwapchainKHR m_swapchain_handler = nullptr;
        VkSurfaceKHR m_surface_handler = nullptr;
        VkFormat m_swapchain_image_format;

        // properties for presentation
        uint32_t m_presentation_index = -1;
        VkQueue m_presentation_queue = nullptr;

        //! @note Render Pass for this swapchain
        VkRenderPass m_swapchain_renderpass = nullptr;
    };
};