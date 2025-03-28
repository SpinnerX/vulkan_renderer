#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {
    buffer_properties create_buffer(uint32_t p_device_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_property_flags) {
        vk_driver driver = vk_driver::driver_context();

        if(driver != nullptr) {
            console_log_warn("driver is still valid!!!");
        }

        buffer_properties new_buffer{};
        new_buffer.AllocateDeviceSize = p_device_size;

        VkBufferCreateInfo buffer_ci = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = new_buffer.AllocateDeviceSize, // size in bytes
            .usage = p_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        // 1. creating our buffer
        vk_check(vkCreateBuffer(driver, &buffer_ci, nullptr, &new_buffer.BufferHandler), "vkCreateBuffer", __FUNCTION__);

        // 2.  getting buffer memory requirements
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(driver, new_buffer.BufferHandler, &memory_requirements);

        // 3. get memory type index
        uint32_t memory_type_index = driver.select_memory_type(memory_requirements.memoryTypeBits, p_property_flags);

        // 4. allocate memory
        /**
         * Memory Type Index
         * - Physical device enumerate all the physical hardware on your machine
         * 
        */
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(vkAllocateMemory(driver, &memory_alloc_info, nullptr, &new_buffer.DeviceMemory), "vkAllocateMemory", __FUNCTION__);

        // 5. bind memory
        vk_check(vkBindBufferMemory(driver, new_buffer.BufferHandler, new_buffer.DeviceMemory, 0), "vkBindBufferMemory", __FUNCTION__);

        return new_buffer;
    }

    /*
    void copy(VkCommandBuffer& p_command_buffer, VkBuffer& p_dst, VkBuffer p_src, uint32_t p_device_size, vk_queue& p_queue) {
        begin_command_buffer(p_command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkBufferCopy buffer_copy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = p_device_size
        };

        vkCmdCopyBuffer(p_command_buffer, p_src, p_dst, 1, &buffer_copy);

        vkEndCommandBuffer(p_command_buffer);

        p_queue.submit_to(p_command_buffer, submission_type::Sync);
        p_queue.wait_idle();
    }
        */

    //! @note In SimpleMesh(in tutorial) only contains vertex buffer and vertex buffer size in bytes
    vk_vertex_buffer::vk_vertex_buffer(const std::span<vertex>& p_vertices) {
        m_driver = vk_driver::driver_context();
        VkDeviceSize device_size_bytes = (sizeof(vertex) * p_vertices.size());

        // 1.) Creating staging buffer
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkMemoryPropertyFlags memory_property_flags =   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        // buffer_properties staging_vertex_buffer = create_buffer(p_vertices.size(), usage, memory_property_flags);
        m_vertex_data = create_buffer(device_size_bytes, usage, memory_property_flags);
        
        // 2. Mapping memory of stage buffer
        void* mapped = nullptr;
        uint32_t offset = 0;
        VkMemoryMapFlags flags = 0;
        vk_check(vkMapMemory(m_driver, m_vertex_data.DeviceMemory, offset, m_vertex_data.AllocateDeviceSize, flags, &mapped), "vkMapMemory", __FUNCTION__);

        // 3. copy vertices of staging buffer
        memcpy(mapped, p_vertices.data(), device_size_bytes);

        // 4. unmap/release memory
        vkUnmapMemory(m_driver, m_vertex_data.DeviceMemory);

        // 5. creating actual final buffer
        // VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        // VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        // m_vertex_data = create_buffer(p_vertices.size(), usage, memory_property_flags);

        // 6. copy staging buffer to final buffer
    }

    void vk_vertex_buffer::copy(const VkCommandBuffer& p_command_buffer) {
    }

    void vk_vertex_buffer::bind(const VkCommandBuffer& p_command_buffer) {
        VkBuffer buffers[] = { m_vertex_data.BufferHandler};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(p_command_buffer, 0, 1, buffers, offsets);

    }

    void vk_vertex_buffer::destroy() {
        // vkDestroyBuffer(m_driver, m_vertex_buffer_handler, nullptr);
        if(m_vertex_data.BufferHandler != nullptr) {
            console_log_warn("vertex buffer handler not nullptr!!!");
        }

        vkFreeMemory(m_driver, m_vertex_data.DeviceMemory, nullptr);
        vkDestroyBuffer(m_driver, m_vertex_data.BufferHandler, nullptr);
        m_vertex_data.AllocateDeviceSize = 0;
        m_vertex_data = {};
    }
};