#pragma once
#include <vulkan/vulkan.h>

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
};