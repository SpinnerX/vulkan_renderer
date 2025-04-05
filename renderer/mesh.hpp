#pragma once
#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_index_buffer.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
    template<>
    struct hash<vk::vertex> {
        size_t operator()(vk::vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.Uv) << 1);
        }
    };
}


namespace vk {

    class mesh {
    public:
        mesh() = default;
        mesh(const std::span<vertex>& p_vertices,
             const std::span<uint32_t>& p_indices);
        mesh(const std::string& p_filename);

        void draw(const VkCommandBuffer& p_cmd_buffer);

        vk_vertex_buffer get_vertex() const { return m_vbo; }
        vk_index_buffer get_index() const { return m_ibo; }

        void destroy();

    private:
        vk_vertex_buffer m_vbo;
        vk_index_buffer m_ibo;
    };
};