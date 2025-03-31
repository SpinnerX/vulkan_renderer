#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan-cpp/vk_queue.hpp>
#include <span>
#include <glm/glm.hpp>
#include <vulkan-cpp/vk_buffer.hpp>

namespace vk {

    // struct vertex {
    //     glm::vec3 Position;
    //     glm::vec4 Color;
    //     glm::vec3 Normals;
    //     glm::vec2 TexCoords;

    //     bool operator==(const vertex& other) const {
    //         return (Position == other.Position and Color == other.Color and
    //                 Normals == other.Normals and TexCoords == other.TexCoords);
    //     }
    // };
    /*
    
        Mesh Class
            - Contains vertex and index buffers
            - As soon we get textures, one thing to consider is allowing staging buffers be used
            - Using staging buffers to ensure CPU can get offloaded to the GPU
    */
    class vk_vertex_buffer {
    public:
        vk_vertex_buffer() = default;
        vk_vertex_buffer(const std::span<vertex>& p_vertices);
        ~vk_vertex_buffer() {}

        void copy(const VkCommandBuffer& p_command_buffer);

        void bind(const VkCommandBuffer& p_command_buffer);

        void draw(const VkCommandBuffer& p_command_buffer);

        size_t count() const { return m_vertices_count; }
        
        size_t size_bytes() const { return m_vertices_byte_size_count; }

        void destroy();

        operator VkBuffer() { return m_vertex_data.BufferHandler; }

        operator VkBuffer() const { return m_vertex_data.BufferHandler; }

        uint32_t size() const { return m_vertex_data.AllocateDeviceSize; }

    private:
        VkDevice m_driver=nullptr;
        uint32_t m_vertices_count=0;
        uint32_t m_vertices_byte_size_count=0;
        buffer_properties m_vertex_data{};
    };
};