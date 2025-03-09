#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>

namespace vk {
    struct swapchain_configs {
        static constexpr uint32_t MaxFramesInFlight = 3;
    };
    class vk_swapchain {
    public:
        // static uint32_t FrameIndex = 0;
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
        VkFramebuffer& read_framebuffer(uint32_t p_frame_index) { return m_swapchain_framebuffers[p_frame_index]; }

        void submit_to(const VkCommandBuffer& p_current_command_buffer);

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

        // Creating synchronization objects
        void create_semaphores();

    public:
        //! @note Acquire Next Image
        uint32_t read_acquired_image();

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
        struct depth_image {
            VkImage Image;
            VkImage ImageView;
            VkDeviceMemory DeviceMemory;
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
        //! @note This swapchain's renderpass
        //! @note Main renderpass to submit rendering tasks to this swapchain, specifically
        VkRenderPass m_renderpass;

        /**
         * @note The only structures that needs to be set to frames in flight of the frames we want to process are framebuffers and images
         * @note The VkFence should be the size of the images available
         * @note This way because if they are all either MaxFramesInFlight including the fences then the fences will try to check the current scene and signaling it
         * @note When you onl want to signal 2 frames at a time, so that the frames do not get ahead of the CPU
        */
        std::array<VkFramebuffer, swapchain_configs::MaxFramesInFlight> m_swapchain_framebuffers;
        // std::vector<VkFramebuffer> m_swapchain_framebuffers;

        //! @note Synchronization Objects
        //! @note Semaphores are for when the frame's available to render to
        std::array<VkSemaphore, swapchain_configs::MaxFramesInFlight> m_swapchain_images_available;
        // std::vector<VkSemaphore> m_swapchain_images_available;

        //! @note Semaphores for when frames rendered are completed
        // std::vector<VkSemaphore> m_swapchain_rendered_images_completed;
        std::vector<VkFence> m_swapchain_fences_in_flight;
        std::array<VkSemaphore, swapchain_configs::MaxFramesInFlight> m_swapchain_rendered_images_completed;

        // VkFence m_current_image_fence = nullptr;


        uint32_t m_image_size = 0;
        uint32_t m_current_frame = 0;
        uint32_t m_current_image_index = 0;
    };
};