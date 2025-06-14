#include <vulkan-cpp/vk_command_buffer.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan/vulkan_core.h>

namespace vk {

    VkCommandBufferLevel to_command_buffer_level(
      const command_buffer_levels& p_level_input) {
        switch (p_level_input) {
            case command_buffer_levels::Primary:
                return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            case command_buffer_levels::Secondary:
                return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            case command_buffer_levels::MaxEnum:
                return VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
        }

        console_log_error("command_buffer_levels was invalid!!!");
    }

    vk_command_buffer::vk_command_buffer(
      const command_buffer_properties& p_properties) {
        m_driver = vk_driver::driver_context();

        VkCommandPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = (VkCommandPoolCreateFlags)p_properties.PoolFlags,
            .queueFamilyIndex = p_properties.QueueIndex
        };

        vk_check(
          vkCreateCommandPool(m_driver, &pool_ci, nullptr, &m_command_pool),
          "vkCreateCommandPool",
          __FUNCTION__);

        VkCommandBufferAllocateInfo command_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_command_pool,
            .level = to_command_buffer_level(p_properties.Levels),
            .commandBufferCount = 1
        };

        vk_check(vkAllocateCommandBuffers(m_driver,
                                          &command_buffer_alloc_info,
                                          &m_command_buffer_handler),
                 "vkAllocateCommandBuffers",
                 __FUNCTION__);

        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };


        vk_check(vkCreateSemaphore(m_driver, 
                                   &semaphore_create_info,
                                   nullptr,
                                   &m_finished_semaphore), 
                 "vkCreateSemaphore",
                 __FUNCTION__);
    }

    void vk_command_buffer::begin(const VkCommandBufferUsageFlags& p_usage) {
        m_begin_end_count++;
        VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = p_usage,
            .pInheritanceInfo = nullptr
        };

        vk_check(vkBeginCommandBuffer(m_command_buffer_handler,
                                      &command_buffer_begin_info),
                 "vkBeginCommandBuffer",
                 __FUNCTION__);
    }

    void vk_command_buffer::end() {
        m_begin_end_count++;

        vkEndCommandBuffer(m_command_buffer_handler);
    }

    void vk_command_buffer::destroy() {
        vkDestroySemaphore(m_driver, m_finished_semaphore, nullptr);
        vkFreeCommandBuffers(
          m_driver, m_command_pool, 1, &m_command_buffer_handler);
        vkDestroyCommandPool(m_driver, m_command_pool, nullptr);
    }
};
