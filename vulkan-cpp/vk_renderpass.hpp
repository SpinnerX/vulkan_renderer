#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {
    class vk_renderpass {
    public:
        vk_renderpass() = default;
        vk_renderpass(const VkDevice& p_driver, const VkSurfaceFormatKHR& p_format);
        ~vk_renderpass();

        operator VkRenderPass() { return m_renderpass_handler; }
        operator VkRenderPass() const { return m_renderpass_handler; }

        void cleanup();

    private:
        VkRenderPass m_renderpass_handler=nullptr;
        VkDevice m_driver=nullptr;
    };
}