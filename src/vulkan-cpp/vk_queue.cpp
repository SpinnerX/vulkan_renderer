#include <vulkan-cpp/vk_queue.hpp>
#include <vulkan-cpp/helper_functions.hpp>

namespace vk {
    static VkSemaphore create_semaphore(const VkDevice& p_driver) {
        // creating semaphores
        VkSemaphoreCreateInfo semaphore_ci = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VkSemaphore semaphore;
        vk_check(vkCreateSemaphore(p_driver, &semaphore_ci, nullptr, &semaphore), "vkCreateSemaphore", __FUNCTION__);
        return semaphore;
    }

    vk_queue::vk_queue(const vk_driver& p_driver, const VkSwapchainKHR& p_swapchain, const VkQueue& p_queue) : m_driver(p_driver), m_swapchain_handler(p_swapchain), m_queue(p_queue) {

        m_render_completed_semaphore = create_semaphore(p_driver);
        m_present_completed_semaphore = create_semaphore(p_driver);
    }

    void vk_queue::submit_to(const VkCommandBuffer& p_command_buffer, submission_type submission_t) {
        VkPipelineStageFlags wait_flags = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit_info = {};
        if(submission_t == submission_type::Async) {
            submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &m_present_completed_semaphore,
                .pWaitDstStageMask = &wait_flags,
                .commandBufferCount = 1,
                .pCommandBuffers = &p_command_buffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &m_render_completed_semaphore
            };   
        }
        else {
            submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = nullptr,
                .commandBufferCount = 1,
                .pCommandBuffers = &p_command_buffer,
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr
            };
        }

        vk_check(vkQueueSubmit(m_queue, 1, &submit_info, nullptr), "vkQueueSubmit", __FUNCTION__);
    }

    void vk_queue::present(uint32_t p_frame_index) {
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_render_completed_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain_handler,
            .pImageIndices = &p_frame_index
        };
        vk_check(vkQueuePresentKHR(m_queue, &present_info), "vkQueuePresentKHR", __FUNCTION__);
    }

    void vk_queue::wait_idle() {
        // vkDeviceWaitIdle(m_driver);
        vkQueueWaitIdle(m_queue);
    }

    uint32_t vk_queue::read_acquire_image() {
        uint32_t image_acquired;
        vk_check(vkAcquireNextImageKHR(m_driver, m_swapchain_handler, UINT64_MAX, m_present_completed_semaphore, nullptr, &image_acquired), "vkAcquireNextImageKHR", __FUNCTION__);
        
        return image_acquired;
    }
    
    void vk_queue::destroy() {
        vkDestroySemaphore(m_driver, m_present_completed_semaphore, nullptr);
        vkDestroySemaphore(m_driver, m_render_completed_semaphore, nullptr);
    }
};