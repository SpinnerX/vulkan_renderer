#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vulkan-cpp/vk_buffer.hpp>
#include <span>

namespace vk {

    VkCommandPool create_single_command_pool();
    VkCommandBuffer create_single_command_buffer(const VkCommandPool& p_command_pool);

    VkCommandBufferBeginInfo commend_buffer_begin_info(const VkCommandBufferUsageFlags& p_usage);

    void begin_command_buffer(const VkCommandBuffer& p_command_buffer, VkCommandBufferUsageFlags p_usage_flags);

    void end_command_buffer(const VkCommandBuffer& p_command_buffer);

    buffer_properties create_buffer(uint32_t p_device_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_property_flags);

    // Use is for vkMap/vkUnmap data of bytes yourself
    void write(const buffer_properties& p_buffer, const void* p_data, size_t p_size_in_bytes);

    // Main use of this would be to vkMap/vkUnmap uint32_t (for indices) and floats (for vertices) data
    void write(const buffer_properties& p_buffer, const std::span<uint32_t>& p_in_buffer);

    void write(const buffer_properties& p_buffer, const std::span<float>& p_in_buffer);

    void write(const buffer_properties& p_buffer, const std::span<vertex>& p_in_buffer);


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