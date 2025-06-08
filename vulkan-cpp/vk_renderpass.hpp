#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/vk_driver.hpp>
#include <span>
namespace vk {
    struct vk_renderpass_options {
        bool cache=false; // set this to true if you want to use the default constructed configuration
        std::span<VkClearColorValue> clear_values{};
        std::span<VkAttachmentDescription> attachments{};
        std::span<VkSubpassDescription> subpass_descriptions{};
        std::span<VkSubpassDependency> dependencies{};
    };

    class vk_renderpass {
    public:
        vk_renderpass() = default;
        vk_renderpass(const vk_renderpass_options& p_options);
        ~vk_renderpass() = default;

        void destroy();

        operator VkRenderPass() const { return m_renderpass_handler; }

        operator VkRenderPass() { return m_renderpass_handler; }

    private:
        VkDevice m_driver=nullptr;
        VkRenderPass m_renderpass_handler = nullptr;
        vk_renderpass_options m_renderpass_options{};
    };
}