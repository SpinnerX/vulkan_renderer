#pragma once
#include "vulkan_physical_driver.hpp"

namespace vk{
    class driver{
    public:
        driver() = default;
        driver(const physical_driver& p_physical);

        uint32_t search_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_properties);

        operator VkDevice(){ return m_device; }

    private:
        VkPhysicalDevice m_physical;
        VkDevice m_device;
    };
};