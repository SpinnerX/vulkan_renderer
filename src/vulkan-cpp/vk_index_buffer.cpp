#include <vulkan-cpp/vk_index_buffer.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {
    vk_index_buffer::vk_index_buffer(const std::span<uint32_t>& p_indices) {
        console_log_info("vk_index_buffer begin initialization!!!");
        m_driver = vk_driver::driver_context();

        m_indices_count = static_cast<uint32_t>(p_indices.size());

        // VkDeviceSize buffer_size = sizeof(p_indices[0]) * m_indices_count;
        VkDeviceSize buffer_size =
          p_indices.size_bytes(); // this should do exactly as what I am
                                  // thinking it does
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkMemoryPropertyFlags property_flags =
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        m_index_buffer_data = create_buffer(buffer_size, usage, property_flags);

        // Does vkMap and vkUnmap
        write(m_index_buffer_data, p_indices);

        console_log_info("vk_index_buffer end initialization successfully!!!!");
    }

    void vk_index_buffer::bind(const VkCommandBuffer& p_command_buffer) {
        vkCmdBindIndexBuffer(p_command_buffer,
                             m_index_buffer_data.BufferHandler,
                             0,
                             VK_INDEX_TYPE_UINT32);
    }

    void vk_index_buffer::draw(const VkCommandBuffer& p_command_buffer) {
        vkCmdDrawIndexed(p_command_buffer, m_indices_count, 1, 0, 0, 0);
    }

    void vk_index_buffer::destroy() {
        vkFreeMemory(m_driver, m_index_buffer_data.DeviceMemory, nullptr);
        vkDestroyBuffer(m_driver, m_index_buffer_data.BufferHandler, nullptr);
    }
};