#include <vulkan-cpp/vk_renderpass.hpp>
#include <vulkan-cpp/helper_functions.hpp>

namespace vk {

    vk_renderpass::vk_renderpass(const VkDevice& p_driver, const VkSurfaceFormatKHR& p_format) : m_driver(p_driver) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = p_format.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        
        // VkRenderPass renderpass;
        vk_check(vkCreateRenderPass(m_driver, &renderPassInfo, nullptr, &m_renderpass_handler), "vkCreateRenderPass", __FUNCTION__);

    }

    vk_renderpass::~vk_renderpass() {
        // vkDestroyRenderPass(m_driver, m_renderpass_handler, nullptr);
        // cleanup();
    }

    void vk_renderpass::cleanup() {
        vkDestroyRenderPass(m_driver, m_renderpass_handler, nullptr);
    }
}