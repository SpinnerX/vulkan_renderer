#include <vulkan-cpp/vk_driver.hpp>
#include <vector>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/helper_functions.hpp>

namespace vk {

    static VkFormat search_supported_depth_format(
      const VkPhysicalDevice& p_physical,
      const std::span<VkFormat>& p_formats,
      VkImageTiling p_tiling,
      VkFormatFeatureFlags p_feature_flag) {
        VkFormat format;

        for (size_t i = 0; i < p_formats.size(); i++) {
            VkFormat current_format = p_formats[i];
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties(
              p_physical, current_format, &format_properties);

            if ((p_tiling == VK_IMAGE_TILING_LINEAR) and
                (format_properties.linearTilingFeatures & p_feature_flag)) {
                format = current_format;
            }
            else if (p_tiling == VK_IMAGE_TILING_OPTIMAL and
                     format_properties.optimalTilingFeatures & p_feature_flag) {
                format = current_format;
            }
        }

        return format;
    }

    static VkFormat search_depth_format(const VkPhysicalDevice& p_physical) {
        std::vector<VkFormat> candidate_formats = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };

        VkFormat format = search_supported_depth_format(
          p_physical,
          candidate_formats,
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return format;
    }

    static VkFormat s_depth_format_selected;

    vk_driver* vk_driver::s_instance = nullptr;
    vk_driver::vk_driver(const vk_physical_driver& p_physical)
      : m_physical_driver(p_physical) {
        console_log_info("vk_driver::vk_driver begin initialization!!!");

        s_depth_format_selected = search_depth_format(p_physical);
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

        vk_check(vkCreateDevice(p_physical, &create_info, nullptr, &m_driver),
                 "vkCreateDevice",
                 __FUNCTION__);

        vkGetDeviceQueue(
          m_driver, graphics_index, 0, &m_device_queues.GraphicsQueue);
        console_log_info("vk_driver::vk_driver end initialization!!!\n\n");

        s_instance = this;
    }

    VkFormat vk_driver::depth_format() {
        return s_depth_format_selected;
    }

    vk_driver::~vk_driver() {}

    void vk_driver::destroy() {
        vkDestroyDevice(m_driver, nullptr);
    }

    VkQueue vk_driver::get_presentation_queue(
      const uint32_t& p_queue_family_index,
      const uint32_t& p_present_queue_index) {
        VkQueue presentation_queue;
        vkGetDeviceQueue(m_driver,
                         p_queue_family_index,
                         p_present_queue_index,
                         &presentation_queue);

        return presentation_queue;
    }

    //! @note Returns -1 if there are no flags available/compatible/valid
    uint32_t vk_driver::select_memory_type(
      uint32_t p_type_filter,
      VkMemoryPropertyFlags p_property_flag) {
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