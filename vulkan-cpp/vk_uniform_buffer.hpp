#pragma once
#include <cstdint>
#include <vulkan-cpp/vk_buffer.hpp>
#include <vector>

namespace vk {
    class vk_uniform_buffer {
    public:
        vk_uniform_buffer(const uint32_t& p_image_size, uint32_t p_size_in_bytes);

        VkBuffer get(uint32_t p_frame_index);

        std::vector<buffer_properties> data() const { return m_uniform_buffers; }

        void destroy();

    private:
        VkDevice m_driver=nullptr;
        std::vector<buffer_properties> m_uniform_buffers{};
    };
};