#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>
#include <vulkan-cpp/vk_queue.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {
    struct swapchain_configs {
        static constexpr uint32_t MaxFramesInFlight = 3;
    };
    class vk_swapchain {
    public:
        // static uint32_t FrameIndex = 0;
        vk_swapchain() = default;
        vk_swapchain(vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface);
        ~vk_swapchain() {}

        void resize(uint32_t p_width, uint32_t p_height);

        template<typename UFunction>
        void record(const UFunction& p_callable) {
            VkClearColorValue clear_color = {1.f, 0.f, 0.f, 0.f};
            VkClearValue clear_value = {};
            clear_value.color = clear_color;

            VkImageSubresourceRange image_range = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

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
                .clearValueCount = 1,
                .pClearValues = &clear_value
            };

            for(uint32_t i = 0; i < m_swapchain_command_buffers.size(); i++) {
                begin_command_buffer(m_swapchain_command_buffers[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

                renderpass_begin_info.framebuffer = m_swapchain_framebuffers[i];

                vkCmdBeginRenderPass(m_swapchain_command_buffers[i], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
                p_callable(m_swapchain_command_buffers[i]);
                vkCmdEndRenderPass(m_swapchain_command_buffers[i]);
                end_command_buffer(m_swapchain_command_buffers[i]);
            }
        }

        vk_queue* current_queue() { return &m_swapchain_queue; }

        void render_scene() {
            // This is needed to ensure that we wait until all commands are executed!
            /**
            @note Something to NOTE: IF you receive an error that involves acquired image being retrieved or a semaphore unsignaled sort of issue, make sure to call this queue.wait_idle!
            */

            // if(m_swapchain_queue.is_resize() == VK_ERROR_OUT_OF_DATE_KHR) {
            //     recreate();
            // }

            m_swapchain_queue.wait_idle();

            uint32_t frame_idx = m_swapchain_queue.read_acquire_image();

            m_swapchain_queue.submit_to(m_swapchain_command_buffers[frame_idx], submission_type::Async);

            m_swapchain_queue.present(frame_idx);
        }

        /*
        template<typename UCallable>
        void submit_to(const UCallable& p_callable){
            uint32_t image_acquired_index = read_acquired_image();

            // This fetches us the current command buffer we are processing
            // While also making this be in use when we are submitting tasks to this
            p_callable(m_swapchain_command_buffers[image_acquired_index]);
        }
        */
        // void submit_to(const VkCommandBuffer& p_current_command_buffer);

        //! @note Acquire Next Image
        uint32_t read_acquired_image();


        // Used to indicate you want to destroy this swapchain
        void destroy();

        // Method used for resizing this swapchain based on window resizing events
        //! TODO: Implement this for swapchain recreation
        void recreate();

        VkRenderPass get_renderpass() const { return m_swapchain_renderpass; }
        VkExtent2D get_extent() const { return m_swapchain_size; }

    private:
        // void begin_command_buffer(const VkCommandBuffer& p_command_buffer, VkCommandBufferUsageFlags p_usage_flags);
        // void end_command_buffer(const VkCommandBuffer& p_command_buffer);
    private:
        //! @note These private functions are for initiating the swapchain first
        void on_create();

        void select_swapchain_surface_formats();

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



        // swapchain internal varioables
        VkPresentModeKHR m_present_mode;
        VkSwapchainKHR m_swapchain_handler;

        // for now command buffers in swapchain
        VkCommandPool m_command_pool = nullptr;
        std::vector<VkCommandBuffer> m_swapchain_command_buffers;

        //! @note Setup Images
        // std::array<image, swapchain_configs::MaxFramesInFlight> m_swapchain_images;
        std::vector<image> m_swapchain_images;

        // swapchain queue
        vk_queue m_swapchain_queue;
        
        VkRenderPass m_swapchain_renderpass=nullptr;
        std::vector<VkFramebuffer> m_swapchain_framebuffers;
    };
};