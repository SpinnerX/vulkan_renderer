#pragma once
#include <array>
#include <deque>
#include <functional>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_queue.hpp>
#include <vulkan-cpp/vk_command_buffer.hpp>
#include <vulkan-cpp/vk_buffer.hpp>

namespace vk {
    struct swapchain_configs {
        static constexpr uint32_t MaxFramesInFlight = 3;
    };
    class vk_swapchain {
    public:
        vk_swapchain() = default;
        vk_swapchain(vk_physical_driver& p_physical,
                     const vk_driver& p_driver,
                     const VkSurfaceKHR& p_surface);
        ~vk_swapchain() {}

        void set_background_color(const std::array<float, 4>& p_color) {
            m_color = { p_color[0], p_color[1], p_color[2], p_color[3] };
        }

        void resize(uint32_t p_width, uint32_t p_height);

        void begin(vk_command_buffer& p_current);
        void end(vk_command_buffer& p_current);

        void submit(const VkCommandBuffer& p_current);
        void present();

        vk_command_buffer get_active_command_buffer(uint32_t p_frame) { return m_swapchain_command_buffers[p_frame]; }

        /*
        //! @note This is something to do once we cleanup the swapchain
        template<typename UCallable>
        void submit_to(const UCallable& p_callable){
            uint32_t image_acquired_index = read_acquired_image();

            // This fetches us the current command buffer we are processing
            // While also making this be in use when we are submitting tasks to
        this p_callable(m_swapchain_command_buffers[image_acquired_index]);
        }
        */
        uint32_t image_size() const { return m_swapchain_images.size(); }

        // Used to indicate you want to destroy this swapchain
        void destroy();

        // Method used for resizing this swapchain based on window resizing
        // events
        //! TODO: Implement this for swapchain recreation
        void recreate();

        //! @note Used by imgui
        VkRenderPass get_renderpass() const { return m_swapchain_renderpass; }

        VkExtent2D get_extent() const { return m_swapchain_size; }

        uint32_t current_frame() const { return m_current_image_index; }

        uint32_t read_acquired_frame();

        VkCommandBuffer current_active_comand_buffer() const {
            return m_swapchain_command_buffers[m_current_image_index].handle();
        }

        static uint32_t image_count() {
            return s_instance->m_swapchain_images.size();
        }

    private:
        //! @note These private functions are for initiating the swapchain first
        void on_create();

    private:
        // change swapchain background color
        VkClearColorValue m_color = { 0.5f, 0.5f, 0.5f, 0.f };

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

        // properties set from physical and logical devices
        VkExtent2D m_swapchain_size;
        surface_properties m_surface_data{};
        VkQueue m_present_queue;

        // submit stuff
        std::deque<std::function<void(VkCommandBuffer)>> m_deletion_stuff;

        // swapchain internal varioables
        VkPresentModeKHR m_present_mode;
        VkSwapchainKHR m_swapchain_handler;

        // for now command buffers in swapchain
        std::vector<vk_command_buffer> m_swapchain_command_buffers;

        //! @note Setup Images
        std::vector<image> m_swapchain_images;
        std::vector<vk_image> m_swapchain_depth_images;

        // swapchain queue
        vk_queue m_swapchain_present_queue;

        VkRenderPass m_swapchain_renderpass = nullptr;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // just to know which image to fetch
        uint32_t m_current_image_index = 0;
    };
};