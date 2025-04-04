#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>
#include <vulkan-cpp/vk_queue.hpp>
#include <deque>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_buffer.hpp>
#include <vulkan-cpp/vk_command_buffer.hpp>

namespace vk {
    struct swapchain_configs {
        static constexpr uint32_t MaxFramesInFlight = 3;
    };
    class vk_swapchain {
    public:
        // static uint32_t FrameIndex = 0;
        vk_swapchain() = default;
        vk_swapchain(vk_physical_driver& p_physical,
                     const vk_driver& p_driver,
                     const VkSurfaceKHR& p_surface);
        ~vk_swapchain() {}

        void set_background_color(const std::array<float, 4>& p_color) {
            m_color = { p_color[0], p_color[1], p_color[2], p_color[3] };
        }

        void resize(uint32_t p_width, uint32_t p_height);

        template<typename UFunction>
        void record(const UFunction& p_callable) {
            console_log_info("vk_swapchain::record Begin recording!!!");
            // VkClearValue clear_value = {};
            // clear_value.color = m_color;
            std::array<VkClearValue, 2> clear_values = {};
            clear_values[0].color = m_color;
            clear_values[1].depthStencil = { 1.0f, 0 };

            // VkImageSubresourceRange image_range = {
            //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            //     .baseMipLevel = 0,
            //     .levelCount = 1,
            //     .baseArrayLayer = 0,
            //     .layerCount = 1
            // };

            VkRenderPassBeginInfo renderpass_begin_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = m_swapchain_renderpass,
                .renderArea = {
                    .offset = {
                        .x = 0,
                        .y = 0
                    },
                    .extent = {
                        .width = m_swapchain_size.width,
                        .height = m_swapchain_size.height
                    },
                },
                // .clearValueCount = 1,
                // .pClearValues = &clear_value
                .clearValueCount = static_cast<uint32_t>(clear_values.size()),
                .pClearValues = clear_values.data()
            };

            for (uint32_t i = 0; i < m_swapchain_command_buffers.size(); i++) {
                m_swapchain_command_buffers[i].begin(
                  VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
                VkViewport viewport = {
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = static_cast<float>(m_swapchain_size.width),
                    .height = static_cast<float>(m_swapchain_size.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                };
                vkCmdSetViewport(
                  m_swapchain_command_buffers[i].handle(), 0, 1, &viewport);

                VkRect2D scissor = {
                    .offset = { 0, 0 },
                    .extent = m_swapchain_size,
                };

                vkCmdSetScissor(
                  m_swapchain_command_buffers[i].handle(), 0, 1, &scissor);

                renderpass_begin_info.framebuffer = m_swapchain_framebuffers[i];

                vkCmdBeginRenderPass(m_swapchain_command_buffers[i],
                                     &renderpass_begin_info,
                                     VK_SUBPASS_CONTENTS_INLINE);
                p_callable(m_swapchain_command_buffers[i].handle());
                vkCmdEndRenderPass(m_swapchain_command_buffers[i]);
                m_swapchain_command_buffers[i].end();
            }

            console_log_info(
              "vk_swapchain::record finished recording successfully!!!");
        }

        vk_queue* current_queue() { return &m_swapchain_queue; }

        //! TODO: Probably want to do this better
        //! @note In the tutorial he has them just after read_acquire_image's
        //! called
        //! @note I put a variable to keep track of our current frame, this will
        //! be used when uniforms are in need to be updated
        template<typename UCallable>
        void update_uniforms(const UCallable& p_callable) {
            p_callable(m_current_image_index);
        }

        void present() {
            // This is needed to ensure that we wait until all commands are
            // executed!
            /**
            @note Something to NOTE: IF you receive an error that involves
            acquired image being retrieved or a semaphore unsignaled sort of
            issue, make sure to call this queue.wait_idle!
            */

            // if(m_swapchain_queue.is_resize() == VK_ERROR_OUT_OF_DATE_KHR) {
            //     recreate();
            // }

            m_swapchain_queue.wait_idle();

            uint32_t frame_idx = m_swapchain_queue.read_acquire_image();

            m_current_image_index = frame_idx;

            m_swapchain_queue.submit_to(
              m_swapchain_command_buffers[frame_idx].handle(),
              submission_type::Async);

            m_swapchain_queue.present(frame_idx);
        }

        /*
        template<typename UCallable>
        void submit_to(const UCallable& p_callable){
            uint32_t image_acquired_index = read_acquired_image();

            // This fetches us the current command buffer we are processing
            // While also making this be in use when we are submitting tasks to
        this p_callable(m_swapchain_command_buffers[image_acquired_index]);
        }
        */
        // void submit_to(const VkCommandBuffer& p_current_command_buffer);

        uint32_t image_size() const { return m_swapchain_images.size(); }

        // Used to indicate you want to destroy this swapchain
        void destroy();

        // Method used for resizing this swapchain based on window resizing
        // events
        //! TODO: Implement this for swapchain recreation
        void recreate();

        VkRenderPass get_renderpass() const { return m_swapchain_renderpass; }

        VkExtent2D get_extent() const { return m_swapchain_size; }

        uint32_t current_frame() const { return m_current_image_index; }

        static VkSurfaceKHR get_surface() {
            return s_instance->m_current_surface;
        }

        // Lets have textures use this????
        //! @note vk_texture should be able to call
        //! vk_swapchain::current_active_comand_buffer() whenever we need to
        //! deal with transition_image_layout
        //! @note vk_texture will essentially be usedf to
        VkCommandBuffer current_active_comand_buffer() const {
            return m_swapchain_command_buffers[m_current_image_index].handle();
        }

        static surface_properties data() { return s_instance->m_surface_data; }

        static uint32_t image_count() {
            return s_instance->m_swapchain_images.size();
        }

        static VkRenderPass swapchain_renderpass() {
            console_log_trace("Orig Rp = {}",
                              (void*)s_instance->m_swapchain_renderpass);
            return s_instance->m_swapchain_renderpass;
        }

        static VkCommandBuffer current_active_buffer() {
            return s_instance
              ->m_swapchain_command_buffers[s_instance->m_current_image_index]
              .handle();
        }

    private:
        //! @note These private functions are for initiating the swapchain first
        void on_create();

        void select_swapchain_surface_formats();

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
        // VkCommandPool m_command_pool = nullptr;
        // std::vector<VkCommandBuffer> m_swapchain_command_buffers;
        std::vector<vk_command_buffer> m_swapchain_command_buffers;

        //! @note Setup Images
        // std::array<image, swapchain_configs::MaxFramesInFlight>
        // m_swapchain_images;
        std::vector<image> m_swapchain_images;
        std::vector<texture_properties> m_swapchain_depth_images;

        // swapchain queue
        vk_queue m_swapchain_queue;

        VkRenderPass m_swapchain_renderpass = nullptr;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;

        // just to know which image to fetch
        uint32_t m_current_image_index = 0;
    };
};