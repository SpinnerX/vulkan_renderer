#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_physical_driver.hpp>

namespace vk {
    class vk_driver {
        struct queue_family_indices {
            uint32_t Graphics = -1;
            uint32_t Compute = -1;
            uint32_t Transfer = -1;
            uint32_t Present = -1;
        };
        struct device_queues {
            VkQueue GraphicsQueue;
            VkQueue TransferQueue;
            VkQueue Compute;
        };
    public:
        vk_driver() = default;
        vk_driver(const vk_physical_driver& p_physical);
        ~vk_driver();

        //! @note Returns -1 if there are no flags available/compatible/valid
        uint32_t select_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_property_flag);

        VkQueue get_graphics_queue() { return m_device_queues.GraphicsQueue; }
        // VkQueue get_presentat_queue() { return ; }

        // This is just for specifically getting presentation queue
        // Now that I think about this, I may as well use this to get our specific queue family from this logical device
        VkQueue get_presentation_queue(const uint32_t& p_present_index, const uint32_t& p_present_queue_index=0);

        static vk_driver& driver_context() { return *s_instance; }

        operator VkDevice() const { return m_driver; }
        operator VkDevice() { return m_driver; }

        void destroy();

    private:
        static vk_driver* s_instance;
        VkDevice m_driver = nullptr;
        vk_physical_driver m_physical_driver;
        VkQueue m_graphics_queue = nullptr;
        device_queues m_device_queues;

        queue_family_indices m_queue_indices;
    };
};