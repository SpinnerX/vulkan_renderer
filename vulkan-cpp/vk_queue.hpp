#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {
    enum class submission_type {
        Sync = 0,
        Async = 1
    };
    class vk_queue {
    public:
        vk_queue() = default;
        vk_queue(const vk_driver& p_driver, const VkSwapchainKHR& p_swapchain, const VkQueue& p_queue);

        void wait_idle();

        uint32_t read_acquire_image();

        /*
        Specify whether you want to submit to the command buffer in either async or synchronization mode
        */
        void submit_to(const VkCommandBuffer& p_command_buffer, submission_type submission_t);

        void present(uint32_t p_frame_index);

        void destroy();
    
    private:

    private:
        vk_driver m_driver;
        VkSwapchainKHR m_swapchain_handler=nullptr;
        VkQueue m_queue = nullptr;
        VkSemaphore m_render_completed_semaphore;
        VkSemaphore m_present_completed_semaphore;
    };
};