#include <renderer/mesh.hpp>
#include <cstdint>
// #include <unordered_map>
#include <xhash>
#include <tiny_obj_loader.h>
#include <vulkan-cpp/logger.hpp>
// #include <renderer/hash.hpp>
#include <glm/glm.hpp>

namespace std {

    // template<>
    // struct hash<vk::vertex> {
    //     size_t operator()(const vk::vertex& vertex) const {
    //         size_t seed = 0;
    //         hash_combine(seed,
    //                             vertex.Position,
    //                             vertex.Color,
    //                             vertex.Normals,
    //                             vertex.TexCoords);
    //         return seed;
    //     }
    // };
    // template <int N, typename T>
    // struct hash<glm::vec<N, T>> {
    //     size_t operator()(const glm::vec<N, T>& v) const {
    //         size_t seed = 0;
    //         for (int i = 0; i < N; ++i) {
    //             seed ^= std::hash<T>()(v.data[i]) + 0x9e3779b9 + (seed << 6) + (seed << 2);
    //         }
    //         return seed;
    //     }
    // };

    // template<> struct hash<vk::new_vertex> {
    //     size_t operator()(vk::new_vertex const& vertex) const {
    //         return ((hash<glm::vec3>()(vertex.Position) ^
    //                (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^
    //                (hash<glm::vec2>()(vertex.TexCoords) << 1);
    //     }
    // };
};

namespace vk {
    mesh::mesh(const std::span<vertex>& p_vertices, const std::span<uint32_t>& p_indices) {
        m_vbo = vk_vertex_buffer(p_vertices);
        m_ibo = vk_index_buffer(p_indices);
    }

    mesh::mesh(const std::string& p_filename) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(
            &attrib, &shapes, &materials, &warn, &err, p_filename.c_str())) {
          console_log_warn("Could not load model from path {}", p_filename);
        }
        else {
            console_log_info("Model Loaded = {}", p_filename);
            // g_loaded_successful = true;
        }

        std::vector<vertex> vertices;
        // std::vector<uint32_t> indices;
        std::vector<uint32_t> indices;

        // std::unordered_map<vertex, uint32_t> unique_vertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                // new_vertex vertex{};

                // if (index.vertex_index >= 0) {
                //     vertex.Position = {
                //         attrib.vertices[3 * index.vertex_index + 0],
                //         attrib.vertices[3 * index.vertex_index + 1],
                //         attrib.vertices[3 * index.vertex_index + 2]
                //     };

                //     vertex.Color = {
                //         attrib.colors[3 * index.vertex_index + 0],
                //         attrib.colors[3 * index.vertex_index + 1],
                //         attrib.colors[3 * index.vertex_index + 2]
                //     };
                // }

                // if (index.normal_index >= 0) {
                //     vertex.Normals = {
                //         attrib.normals[3 * index.normal_index + 0],
                //         attrib.normals[3 * index.normal_index + 1],
                //         attrib.normals[3 * index.normal_index + 2]
                //     };
                // }

                // if (index.texcoord_index >= 0) {
                //     vertex.TexCoords = {
                //         attrib.texcoords[2 * index.texcoord_index + 0],
                //         attrib.texcoords[2 * index.texcoord_index + 1]
                //     };
                // }

                // vertices.push_back(vertex);
                // if (unique_vertices.contains(vertex) == 0) {
                //     unique_vertices[vertex] =
                //       static_cast<uint32_t>(vertices.size());
                //     vertices.push_back(vertex);
                // }

                // indices.push_back(unique_vertices[vertex]);
            }
        }

        // m_vbo = vk_vertex_buffer(vertices);
        // m_ibo = vk_index_buffer(indices);
    }

    void mesh::draw(const VkCommandBuffer& p_cmd_buffer) {
        m_vbo.bind(p_cmd_buffer);

        if(m_ibo.has_indices()) {
            m_ibo.bind(p_cmd_buffer);
            m_ibo.draw(p_cmd_buffer);
        }
        else {
            
        }
    }
};