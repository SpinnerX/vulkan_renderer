#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <array>

namespace vk {
    vk_swapchain* vk_swapchain::s_instance = nullptr;
    VkPresentModeKHR select_compatible_present_mode(
        const VkPresentModeKHR& p_request,
        const std::vector<VkPresentModeKHR>& p_modes) {
        for (const auto& mode : p_modes) {
            if (mode == p_request) {
                return mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // validate the capabilities to ensure we are not requesting the maximum over the amount of images we are able to request
    uint32_t select_images_size(const VkSurfaceCapabilitiesKHR& p_surface_capabilities) {
        uint32_t requested_images = p_surface_capabilities.minImageCount + 1;

        uint32_t final_image_count = 0;

        if((p_surface_capabilities.maxImageCount > 0) and (requested_images > p_surface_capabilities.maxImageCount)) {
            final_image_count = p_surface_capabilities.maxImageCount;
        }
        else {
            final_image_count = requested_images;
        }

        return final_image_count;
    }

    vk_swapchain::vk_swapchain(vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface) : m_driver(p_driver), m_current_surface(p_surface) {
        m_surface_data = p_physical.get_surface_properties(p_surface);

        m_swapchain_size = m_surface_data.SurfaceCapabilities.currentExtent;

        // request what our minimum image count is
        uint32_t request_min_image_count = select_images_size(m_surface_data.SurfaceCapabilities);


        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = p_surface,
            .minImageCount = request_min_image_count,
            .imageFormat = m_surface_data.SurfaceFormat.format,
            .imageColorSpace = m_surface_data.SurfaceFormat.colorSpace,
            // use physical device surface formats to getting the right formats in vulkan
            .imageExtent = m_swapchain_size,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = m_surface_data.SurfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
            .clipped = true
        };

        vk_check(vkCreateSwapchainKHR(m_driver, &swapchain_ci, nullptr, &m_swapchain_handler), "vkCreateSwapchainKHR", __FUNCTION__);

        // querying images we have

        uint32_t count=0;
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &count, nullptr);
        std::vector<VkImage> images(count);
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &count, images.data());

    }

    void vk_swapchain::destroy() {
        vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
    }

    uint32_t vk_swapchain::read_acquired_image() {
        uint32_t image_index;

        vkAcquireNextImageKHR(m_driver, m_swapchain_handler, std::numeric_limits<uint32_t>::max(), m_swapchain_images_available[m_current_frame], VK_NULL_HANDLE, &image_index);
        m_current_image_index = image_index;
        return image_index;
    }

    void vk_swapchain::resize(uint32_t p_width, uint32_t p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }


};