#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace vk {
    struct buffer_properties {
        VkBuffer BufferHandler=nullptr;
        VkDeviceMemory DeviceMemory=nullptr;
        uint32_t AllocateDeviceSize=0;
    };

    struct image_data {
        VkImage Image=nullptr;
        VkImageView ImageView=nullptr;
        VkSampler Sampler=nullptr;
        VkDeviceMemory DeviceMemory=nullptr;
        uint32_t Width=0;
        uint32_t Height=0;
    };

    struct texture_properties {
        VkImage Image=nullptr;
        VkImageView ImageView=nullptr;
        VkSampler Sampler=nullptr;
        VkDeviceMemory DeviceMemory=nullptr;
        uint32_t AllocateDeviceSize=0;
    };

    // struct vertex {
    //     glm::vec3 Position;
    //     glm::vec2 Uv;
    // };
    // struct vertex {
    //     // vertex() = default;
    //     glm::vec3 Position;
    //     glm::vec3 Color;
    //     glm::vec3 Normals;
    //     glm::vec2 TexCoords;

    //     bool operator==(const vertex& other) const {
    //         return (Position == other.Position and Color == other.Color and
    //                 Normals == other.Normals and TexCoords == other.TexCoords);
    //     }
    // };

    struct vertex {
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec3 Normals;
        glm::vec2 Uv;

        bool operator==(const vertex& other) const {
            return Position == other.Position && Color == other.Color && Uv == other.Uv;
        }
    };

    struct new_vertex {
        glm::vec3 Position;
        glm::vec2 TexCoords;
        glm::vec3 Color;
    };
};