#include "vulkan_physical_driver.hpp"
#include <fmt/core.h>
#include <vulkan/vulkan_core.h>
#include "helper_functions.hpp"

namespace vk{
    physical_driver::physical_driver(const VkInstance& p_instance){
        fmt::println("Vulkan Begin Physical Driver Initialization!!");

        uint32_t gpu_count = -1;
        std::vector<VkPhysicalDevice> available_gpus;
        //! @note Querying number of available GPU's (Physical Devices) on our machine
        vk_check(vkEnumeratePhysicalDevices(p_instance, &gpu_count, nullptr), "vkEnumeratePhysicalDevices", __FILE__, __LINE__, __FUNCTION__);

        available_gpus.resize(gpu_count);
        vk_check(vkEnumeratePhysicalDevices(p_instance, &gpu_count, available_gpus.data()), "vkEnumeratePhysicalDevices", __FILE__, __LINE__, __FUNCTION__);


        for(const auto& device : available_gpus){
            VkPhysicalDeviceProperties device_properties;
            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceProperties(device, &device_properties);
            vkGetPhysicalDeviceFeatures(device, &device_features);

            //! @note VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU checks if our device is an external device.
            if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
                m_physical_handler = device;
                break;
            }
        }

        //! @note Wanting to ensure that the available GPU's that we search for are valid
        if(available_gpus.size() == 0){
            m_physical_handler = available_gpus[0];
        }

        if(available_gpus.empty()){
            fmt::println("There was no available compatible GPU's found!");
            m_physical_handler = VK_NULL_HANDLE; 
            return;
        }

        //! @note Initialized and Identifying Queue Families.
        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_handler, &queue_family_count, nullptr);
        m_queue_family_properties.resize(queue_family_count);

        //! @note Loading queue family data from the physical device that we have selected.
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_handler, &queue_family_count, m_queue_family_properties.data());


        //! @note Getting our actual physical device properties that have been currently selected.
        vkGetPhysicalDeviceProperties(m_physical_handler, &m_device_properties);

        //! @note Getting our physical device memory proeprties to know what memory properties are available to us
        // m_queue_family_indices = get_queue_family_indices(); // replacing this function that gets called here
        uint32_t i = 0;
        for(const auto&    queue_family : m_queue_family_properties){
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                m_queue_family_indices.Graphics = i;
                break;
            }
            i++;
        }

        fmt::println("Vulkan Succesfully Initializing Physical Driver");
    }

    physical_driver::queue_family_indices physical_driver::get_queue_family_indices(){
        return m_queue_family_indices;
    }

    uint32_t physical_driver::search_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_properties){
        VkPhysicalDeviceMemoryProperties device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(m_physical_handler, &device_memory_properties);

        for(uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++){
            for(uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++){
                if((p_type_filter & (1 << i)) and (device_memory_properties.memoryTypes[i].propertyFlags & p_properties)){
                    return i;
                }
            }
        }
        return -1;
    }
};