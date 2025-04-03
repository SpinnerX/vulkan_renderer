#pragma once
#include <vulkan/vulkan.h>

namespace vk {

    enum command_buffer_levels : uint8_t {
        Primary = 0,
        Secondary = 1,
        MaxEnum = 2
    };

    struct command_buffer_properties {
        command_buffer_properties(
          uint32_t p_queue_family,
          const command_buffer_levels& p_levels,
          const VkCommandPoolCreateFlagBits& p_pool_flags)
          : QueueIndex(p_queue_family)
          , Levels(p_levels)
          , PoolFlags(p_pool_flags) {}

        command_buffer_levels Levels;
        uint32_t QueueIndex = -1;
        VkCommandPoolCreateFlagBits PoolFlags;
    };

    /*

        Wrapper around vulkan's command buffer use

        should also be able to utilize this class as the following

        command_buffer_properties properties = {
            .p_queue_family = graphics_index,
            .p_levels = command_buffer_levels::Primary
        };
        vk_command_buffer command_buffer = vk_command_buffer(properties);


        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        command_buffer.end();
    */
    class vk_command_buffer {
    public:
        vk_command_buffer() = default;
        vk_command_buffer(const command_buffer_properties& p_properties);

        // begin recording to this command buffer
        void begin(const VkCommandBufferUsageFlags& p_usage);

        // end recording to this command buffer
        void end();

        VkCommandBuffer handle() const { return m_command_buffer_handler; }

        void destroy();

        // checks if command buffer is valid because we also check if begin and
        // end are setup correctly
        bool is_valid_operation() const { return (m_begin_end_count == 2); }

        // checks if VkCommandBuffer is valid
        bool is_handle_valid() const {
            return (m_command_buffer_handler != nullptr);
        }

        operator VkCommandBuffer() { return m_command_buffer_handler; }
        operator VkCommandBuffer() const { return m_command_buffer_handler; }

    private:
        VkDevice m_driver;
        // This is just to indicate to throw an error if you call end before
        // begin or begin and no end for this command buffer To at least give us
        // an indication if there is no begin/end being called for this specific
        // command buffer If begin/end are both called respectively, this
        // counter should be 2.
        uint32_t m_begin_end_count = 0;
        VkCommandPool m_command_pool = nullptr;
        VkCommandBuffer m_command_buffer_handler = nullptr;
    };

};