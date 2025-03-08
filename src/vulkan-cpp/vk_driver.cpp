#include <vulkan-cpp/vk_driver.hpp>
#include <vector>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/helper_functions.hpp>

namespace vk {
    vk_driver::vk_driver(const vk_physical_driver& p_physical) : m_physical_driver(p_physical) {
        console_log_info("vk_driver::vk_driver begin initialization!!!");
        float queue_priority[1] = { 0.0f };
        /*

            VK_EXT_descriptor_buffer
                - Simplifies and maps more directly to how hardware sees
           descriptors
                - Simplifies the programming model, by not needing to create
           descriptor pools up front
        */
        std::vector<const char*> device_extension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        uint32_t graphics_index = p_physical.get_queue_indices().Graphics;

        console_log_trace("Graphics Queue Indices = {}", graphics_index);

        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphics_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priority,
        };

        VkDeviceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_create_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount =
              static_cast<uint32_t>(device_extension.size()),
            .ppEnabledExtensionNames = device_extension.data(),
        };

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(p_physical, &features);
        features.robustBufferAccess = false;
        create_info.pEnabledFeatures = &features;

        vk_check(vkCreateDevice(p_physical, &create_info, nullptr, &m_driver), "vkCreateDevice", __FUNCTION__);

        vkGetDeviceQueue(m_driver, graphics_index, 0, &m_device_queues.GraphicsQueue);
        console_log_info("vk_driver::vk_driver end initialization!!!\n\n");
    }


    vk_driver::~vk_driver(){
        vkDestroyDevice(m_driver, nullptr);
    }

    //! @note Returns -1 if there are no flags available/compatible/valid
    uint32_t vk_driver::select_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_property_flag){
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(m_physical_driver, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((p_type_filter & (1 << i)) and
                (mem_props.memoryTypes[i].propertyFlags & p_property_flag) ==
                  p_property_flag) {
                return i;
            }
        }

        return -1;
    }
};