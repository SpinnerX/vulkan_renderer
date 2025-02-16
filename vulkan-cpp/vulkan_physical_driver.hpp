#pragma once
#include <vulkan/vulkan.hpp>

namespace vk{
    class physical_driver{
        struct queue_family_indices{
            uint32_t Graphics = -1;
            uint32_t Compute = -1;
            uint32_t Transfer = -1;
        };
    public:
        physical_driver() = default;
        physical_driver(const VkInstance& p_instance);

        queue_family_indices get_queue_indices() const { return m_queue_family_indices; }
        VkPhysicalDeviceProperties get_device_properties() const { return m_device_properties; }
        
        uint32_t search_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_properties);

        std::vector<VkQueueFamilyProperties> get_queue_family_properties() const { return m_queue_family_properties; }

        operator VkPhysicalDevice(){ return m_physical_handler; }
        operator VkPhysicalDevice() const{ return m_physical_handler; }
    private:
        
        queue_family_indices get_queue_family_indices();

    private:

        VkPhysicalDevice m_physical_handler;
        VkPhysicalDeviceProperties m_device_properties;
        std::vector<VkQueueFamilyProperties> m_queue_family_properties;
        queue_family_indices m_queue_family_indices;
    };
};