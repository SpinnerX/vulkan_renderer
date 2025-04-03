#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {

    //! @note In SimpleMesh(in tutorial) only contains vertex buffer and vertex
    //! buffer size in bytes
    vk_vertex_buffer::vk_vertex_buffer(const std::span<vertex>& p_vertices) {
        m_driver = vk_driver::driver_context();
        m_vertices_count = static_cast<uint32_t>(p_vertices.size());
        m_vertices_byte_size_count = p_vertices.size_bytes();

        // 1.) Creating staging buffer
        //! @note Validation layers throw an error when
        //! VK_BUFFER_USAGE_STORAGE_BUFFER_BIT isn't set for VkBuffer (I bitwise
        //! OR this just to silence that validation error for now)
        //! @note TODO: Need to setup ways for staging buffers to work with the
        //! current command buffer being worked on
        //! @note VK_BUFFER_USAGE_STORAGE_BUFFER_BIT is required for vertex
        //! buffer by descriptor set
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkMemoryPropertyFlags memory_property_flags =
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        // buffer_properties staging_vertex_buffer =
        // create_buffer(p_vertices.size(), usage, memory_property_flags);
        m_vertex_data = create_buffer(
          m_vertices_byte_size_count, usage, memory_property_flags);

        // 2. Mapping memory of vertex buffer (vkMap/vkUnmap operation)
        write(m_vertex_data, p_vertices);
    }

    void vk_vertex_buffer::copy(const VkCommandBuffer& p_command_buffer) {}

    void vk_vertex_buffer::bind(const VkCommandBuffer& p_command_buffer) {
        VkBuffer buffers[] = { m_vertex_data.BufferHandler };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(p_command_buffer, 0, 1, buffers, offsets);
    }

    void vk_vertex_buffer::draw(const VkCommandBuffer& p_command_buffer) {
        vkCmdDraw(p_command_buffer, m_vertices_count, 1, 0, 0);
    }

    void vk_vertex_buffer::destroy() {
        vkFreeMemory(m_driver, m_vertex_data.DeviceMemory, nullptr);
        vkDestroyBuffer(m_driver, m_vertex_data.BufferHandler, nullptr);
        m_vertex_data.AllocateDeviceSize = 0;
        m_vertex_data = {};
    }
};