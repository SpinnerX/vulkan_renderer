#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {

    // Creating a single uniform buffer through these helper functions
    buffer_properties create_uniform_buffer(uint32_t p_size) {

        VkBufferUsageFlags flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkMemoryPropertyFlags memory_property_flag =
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        buffer_properties buffer_data =
          create_buffer(p_size, flags, memory_property_flag);

        return buffer_data;
    }

    vk_uniform_buffer::vk_uniform_buffer(uint32_t p_size_in_bytes) {

        m_driver = vk_driver::driver_context();

        console_log_info("vk_uniform_buffer begin initialization!!!");
        // m_uniform_buffers.resize(p_image_size);

        // for(size_t i = 0; i < m_uniform_buffers.size(); i++) {
        //     m_uniform_buffers[i] = create_uniform_buffer(p_size_in_bytes);
        // }
        m_uniform_buffer_data = create_uniform_buffer(p_size_in_bytes);

        console_log_info(
          "vk_uniform_buffer successfully finished initialization!!!\n\n");
    }

    VkBuffer vk_uniform_buffer::get(uint32_t p_frame_index) {
        // return m_uniform_buffers[p_frame_index].BufferHandler;
        return m_uniform_buffer_data.BufferHandler;
    }

    void vk_uniform_buffer::update(const void* p_data, size_t p_size_in_bytes) {

        void* mapped = nullptr;
        vk_check(vkMapMemory(m_driver,
                             m_uniform_buffer_data.DeviceMemory,
                             0,
                             p_size_in_bytes,
                             0,
                             &mapped),
                 "vkMapMemory",
                 __FUNCTION__);
        memcpy(mapped, p_data, p_size_in_bytes);
        vkUnmapMemory(m_driver, m_uniform_buffer_data.DeviceMemory);
    }

    void vk_uniform_buffer::destroy() {
        vkFreeMemory(m_driver, m_uniform_buffer_data.DeviceMemory, nullptr);
        vkDestroyBuffer(m_driver, m_uniform_buffer_data.BufferHandler, nullptr);
    }
};