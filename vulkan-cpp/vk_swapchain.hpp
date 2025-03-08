#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>

namespace vk {
    struct swapchain_configs {
        static constexpr uint32_t MaxFramesInFlight = 2;
    };
    class vk_swapchain {
    public:
        vk_swapchain() = default;
        vk_swapchain(const vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface);
        ~vk_swapchain();

        void resize(uint32_t p_width, uint32_t p_height);

        static VkExtent2D get_extent() { return s_instance->m_swapchain_size; }

        static VkRenderPass get_renderpass() { return s_instance->m_renderpass; }

        operator VkRenderPass() { return m_renderpass; }
        operator VkRenderPass() const { return m_renderpass; }

        VkSurfaceFormatKHR get_format() const { return m_surface_format; }

        VkFramebuffer read_framebuffer(uint32_t p_frame_index) const { return m_swapchain_framebuffers[p_frame_index]; }

    private:
        //! @note These private functions are for initiating the swapchain first
        void on_create();

        void select_swapchain_surface_formats();

        void select_presentation_mode();

        void get_valid_extent();


        // Now we create a renderpass.
        //! @note At the time I made this function, I implemented the shader class first beforehand
        void create_renderpass_for_swapchain();

        // Now we are creating framebuffers
        void create_swapchain_framebuffers();

    private:
        // private functions for initiating the VkImage/VkImageView
        /**
         * @note Sets up and prepares our image for swapchain
        */
        void setup_images();

    private:
        static vk_swapchain* s_instance;
        vk_physical_driver m_physical;
        vk_driver m_driver;
        VkSurfaceKHR m_current_surface;

        struct image {
            VkImage Image;
            VkImageView ImageView;
        };

        // surface properties
        VkSurfaceCapabilitiesKHR m_surface_capabilities;
        VkExtent2D m_swapchain_size;
        VkSurfaceFormatKHR m_surface_format;

        // presentation mode
        VkPresentModeKHR m_present_mode;
        uint32_t m_present_index = -1;
        VkQueue m_present_queue;

        VkSwapchainKHR m_swapchain_handler;

        //! @note Setup Images
        std::array<image, swapchain_configs::MaxFramesInFlight> m_swapchain_images;

        //! @note Setting up Renderpass
        VkRenderPass m_renderpass;

        std::array<VkFramebuffer, swapchain_configs::MaxFramesInFlight> m_swapchain_framebuffers;
    };
};