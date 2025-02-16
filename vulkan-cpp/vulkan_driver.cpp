#include "vulkan_driver.hpp"
#include "helper_functions.hpp"
#include <fmt/core.h>

namespace vk{
    driver::driver(const physical_driver& p_physical) : m_physical(p_physical){
        fmt::println("Vulkan Begin Logical Driver Initialization!!!");
        float queue_priority[1] = { 0.0f };
        /*
        
            VK_EXT_descriptor_buffer
                - Simplifies and maps more directly to how hardware sees descriptors
                - Simplifies the programming model, by not needing to create descriptor pools up front
        */
        std::vector<const char*> device_extension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VkDeviceQueueCreateInfo device_queue_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_physical.get_queue_indices().Graphics,
            .queueCount = 1,
            .pQueuePriorities = queue_priority,
        };

        VkDeviceCreateInfo device_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &device_queue_ci,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(device_extension.size()),
            .ppEnabledExtensionNames = device_extension.data(),
        };

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(m_physical, &features);
        features.robustBufferAccess = false;
        device_ci.pEnabledFeatures = &features;

        vk_check(vkCreateDevice(m_physical, &device_ci, nullptr, &m_device), "vkCreateDevice", __FILE__, __LINE__, __FUNCTION__);
        
        fmt::println("Vulkn successfully initialized logical driver!!");
    }

    uint32_t driver::search_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_properties){
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(m_physical, &mem_props);

        for(uint32_t i = 0; i < mem_props.memoryTypeCount; i++){
            if((p_type_filter & (1 << i)) and (mem_props.memoryTypes[i].propertyFlags & p_properties) == p_properties){
                return i;
            }
        }

        return -1;
    }
};