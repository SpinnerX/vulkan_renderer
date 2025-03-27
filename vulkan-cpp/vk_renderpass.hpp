#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {
    class vk_renderpass {
    public:
        vk_renderpass() = default;
        // vk_renderpass();
        ~vk_renderpass() {}

        void destroy() {}

    private:
        VkRenderPass m_renderpass_handler=nullptr;
    };
}