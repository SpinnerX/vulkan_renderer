#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vk {
    struct surface_properties {
        VkSurfaceCapabilitiesKHR SurfaceCapabilities;
        VkSurfaceFormatKHR SurfaceFormat;
    };

    class vk_physical_driver {
        struct queue_family_indices {
            uint32_t Graphics = -1;
            uint32_t Compute = -1;
            uint32_t Transfer = -1;
            uint32_t Present = -1;
        };
    public:
        vk_physical_driver() = default;
        vk_physical_driver(const VkInstance& p_instance);
        ~vk_physical_driver();

        operator VkPhysicalDevice() const { return m_physical_driver; }
        operator VkPhysicalDevice() { return m_physical_driver; }

        queue_family_indices get_queue_indices() const { return m_queue_indices; }


        uint32_t get_presentation_index(const VkSurfaceKHR& p_surface);

        // VkSurfaceCapabilitiesKHR get_surface_capabilities();

        // VkSurfaceFormatKHR get_current_surface_format(const VkSurfaceKHR& p_surface);

        /*
            Physical Device should provide the surface properties to other contexts of vulkan like the swapchain
        */
        surface_properties get_surface_properties(const VkSurfaceKHR& p_surface);


    private:
        queue_family_indices select_queue_family_indices();


    private:
        VkPhysicalDevice m_physical_driver = nullptr;
        queue_family_indices m_queue_indices{};
        std::vector<VkQueueFamilyProperties> m_queue_family_properties;
        surface_properties m_surface_properties{};
    };
};