#pragma once
#include <vulkan/vulkan.h>

namespace vk {
    struct buffer_properties {
        VkBuffer BufferHandler=nullptr;
        VkDeviceMemory DeviceMemory=nullptr;
        uint32_t AllocateDeviceSize=0;
    };
};