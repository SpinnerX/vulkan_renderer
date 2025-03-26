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

        operator VkDevice() const { return m_driver; }
        operator VkDevice() { return m_driver; }

        void clean();

    private:
        VkDevice m_driver = nullptr;
        vk_physical_driver m_physical_driver;
        VkQueue m_graphics_queue = nullptr;
        device_queues m_device_queues;

        queue_family_indices m_queue_indices;
    };
};