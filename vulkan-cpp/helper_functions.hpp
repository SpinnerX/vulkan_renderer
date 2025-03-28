#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vulkan-cpp/vk_buffer.hpp>

namespace vk {
    void begin_command_buffer(const VkCommandBuffer& p_command_buffer, VkCommandBufferUsageFlags p_usage_flags);

    void end_command_buffer(const VkCommandBuffer& p_command_buffer);

    buffer_properties create_buffer(uint32_t p_device_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_property_flags);


    // const char* vk_to_string(VkResult res);
    //! @note Terminates if the result was unsuccessful!
    //! @note TODO --- We shouldn't std::terminate, look into alternatives.
    void vk_check(const VkResult& p_result,
                  const char* p_tag,
                  const char* p_function_name,
                  const char* p_filepath = __FILE__,
                  uint32_t p_line = __LINE__);

    void vk_check_format(VkFormat p_format,
                        const char* p_function_name,
                        const char* p_filepath = __FILE__,
                        uint32_t p_line=__LINE__);

    std::string vk_queue_flags_to_string(VkQueueFlagBits p_flags);

    std::string vk_present_mode_to_string(VkPresentModeKHR p_present_mode);

    std::string vk_format_to_string(VkFormat p_format);
};