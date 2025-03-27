#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>
#include <vulkan-cpp/vk_queue.hpp>

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

        
        void record() {
            VkClearColorValue clear_color = {1.f, 0.f, 0.f, 0.f};

            VkImageSubresourceRange image_range = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            for(uint32_t i = 0; i < m_swapchain_command_buffers.size(); i++) {
                // vkResetCommandBuffer(m_swapchain_command_buffers[i], 0);

                VkImageMemoryBarrier present_to_clear_barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_swapchain_images[i].Image,
                    .subresourceRange = image_range
                };

                VkImageMemoryBarrier clear_to_present_barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_swapchain_images[i].Image,
                    .subresourceRange = image_range
                };

                begin_command_buffer(m_swapchain_command_buffers[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

                vkCmdPipelineBarrier(
                    m_swapchain_command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &present_to_clear_barrier
                );

                vkCmdClearColorImage(m_swapchain_command_buffers[i], m_swapchain_images[i].Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_range);

                vkCmdPipelineBarrier(
                    m_swapchain_command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &clear_to_present_barrier
                );

                end_command_buffer(m_swapchain_command_buffers[i]);
            }
        }

        void render_scene() {
            // This is needed to ensure that we wait until all commands are executed!
            m_swapchain_queue.wait_idle();

            uint32_t frame_idx = m_swapchain_queue.read_acquire_image();

            m_swapchain_queue.submit_async(m_swapchain_command_buffers[frame_idx]);

            m_swapchain_queue.present(frame_idx);
        }

        // void submit_to(const VkCommandBuffer& p_current_command_buffer);

        /*
        template<typename UCallable>
        void submit_to(const UCallable& p_callable){
            uint32_t image_acquired_index = read_acquired_image();

            // This fetches us the current command buffer we are processing
            // While also making this be in use when we are submitting tasks to this
            p_callable(m_swapchain_command_buffers[image_acquired_index]);
        }
        */

        //! @note Acquire Next Image
        uint32_t read_acquired_image();


        // Used to indicate you want to destroy this swapchain
        void destroy();

        // Method used for resizing this swapchain based on window resizing events
        //! TODO: Implement this for swapchain recreation
        void recreate() {}
    private:
        void begin_command_buffer(const VkCommandBuffer& p_command_buffer, VkCommandBufferUsageFlags p_usage_flags);
        void end_command_buffer(const VkCommandBuffer& p_command_buffer);
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

    };
};