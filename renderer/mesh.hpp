#pragma once
#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_index_buffer.hpp>

namespace vk {

    class mesh {
    public:
        mesh() = default;
        mesh(const std::span<vertex>& p_vertices, const std::span<uint32_t>& p_indices);
        mesh(const std::string& p_filename);

        void draw(const VkCommandBuffer& p_cmd_buffer);


    private:
        vk_vertex_buffer m_vbo;
        vk_index_buffer m_ibo;
    };
};