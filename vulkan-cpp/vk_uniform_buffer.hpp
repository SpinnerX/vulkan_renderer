#pragma once
#include <cstdint>
#include <vulkan-cpp/vk_buffer.hpp>
#include <vector>

namespace vk {
    class vk_uniform_buffer {
    public:
        vk_uniform_buffer() = default;
        vk_uniform_buffer(uint32_t p_size_in_bytes);

        VkBuffer get(uint32_t p_frame_index);

        void update(const void* p_data, size_t p_size_in_bytes);

        operator VkBuffer() { return m_uniform_buffer_data.BufferHandler; }

        operator VkBuffer() const {
            return m_uniform_buffer_data.BufferHandler;
        }

        // std::vector<buffer_properties> data() const { return
        // m_uniform_buffers; }

        void destroy();

    private:
        VkDevice m_driver = nullptr;
        buffer_properties m_uniform_buffer_data{};
        // VkDeviceMemory m_device_memory=nullptr;
        // std::vector<buffer_properties> m_uniform_buffers{};
    };
};