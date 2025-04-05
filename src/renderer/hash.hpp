#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan-cpp/vk_vertex_buffer.hpp>

namespace std {
    template<>
    struct hash<vk::vertex> {
        size_t operator()(vk::vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec4>()(vertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.Uv) << 1);
        }
    };
}