#pragma once
#include <span>
#include <vulkan-cpp/vk_buffer.hpp>

namespace vk {
    class vk_index_buffer {
    public:
        vk_index_buffer() = default;
        vk_index_buffer(const std::span<uint32_t>& p_indices);

        ~vk_index_buffer() {}


        uint32_t count() const { return m_indices_count; }

        bool has_indices() const { return (m_indices_count > 0); }

        void bind(const VkCommandBuffer& p_command_buffer);

        void draw(const VkCommandBuffer& p_command_buffer);

        void destroy();

    private:
        VkDevice m_driver=nullptr;
        buffer_properties m_index_buffer_data{};
        uint32_t m_indices_count=0;
    };
};