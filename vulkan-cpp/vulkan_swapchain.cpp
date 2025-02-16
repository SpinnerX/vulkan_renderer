#include "vulkan_swapchain.hpp"
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <vulkan/vulkan_core.h>
#include "helper_functions.hpp"

namespace vk{
    static uint32_t get_presentation_index(const physical_driver& p_physical, const VkSurfaceKHR& p_Surface){
        uint32_t presentation_index = -1;
        VkBool32 compatible = VK_FALSE;
        uint32_t i = 0;
        for(const auto& queue_family : p_physical.get_queue_family_properties()){
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                vk_check(vkGetPhysicalDeviceSurfaceSupportKHR(p_physical, i, p_Surface, &compatible), "vkGetPhysicalDeviceSurfaceSupportKHR", __FILE__, __LINE__, __FUNCTION__);

                if(compatible){
                    presentation_index = i;
                }
            }
            i++;
        }

        // ConsoleLogWarn("Presentation Index === {}", PresentationIndex);
        fmt::println("Presentation Index === {}", presentation_index);
        return presentation_index;
    }
    swapchain::swapchain(const physical_driver& p_physical, const driver& p_driver, const VkSurfaceKHR& p_surface) : m_physical(p_physical), m_driver(p_driver), m_surface_handler(p_surface){
        fmt::println("Vulkan Begin Swapchain Initialization!!!");
        m_presentation_index = get_presentation_index(m_physical, m_surface_handler);
        
        vkGetDeviceQueue(m_driver, m_presentation_index, 0, &m_presentation_queue);

        // on_swapchain_create();
        // on_create();
        fmt::println("Vulkan Successfully with Swapchain Initialization!!!");
    }

    void swapchain::on_create(uint32_t p_width, uint32_t p_height){
        fmt::println("Swapchain OnCreate Begin Initialization!!");

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical, m_surface_handler, &m_surface_capabilities);

        on_swapchain_format_query(m_physical);
        on_swapchain_presentation_query();

        //! @note Setting up Swapchain Extents
        if(m_surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
            m_swapchain_extent = m_surface_capabilities.currentExtent;
        }
        else{
            VkExtent2D actual_extent = {
                .width = p_width,
                .height = p_height
            };

            actual_extent.width = std::clamp(actual_extent.width, m_surface_capabilities.minImageExtent.width, m_surface_capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(actual_extent.height, m_surface_capabilities.minImageExtent.height, m_surface_capabilities.maxImageExtent.height);
        
            m_swapchain_extent = actual_extent;
        }

        uint32_t image_count = m_surface_capabilities.minImageCount + 1;
        if(m_surface_capabilities.maxImageCount > 0 && image_count > m_surface_capabilities.maxImageCount){
            image_count = m_surface_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_surface_handler,

            .minImageCount = image_count,
            .imageFormat = m_current_selected_surface_format.format,
            .imageColorSpace = m_current_selected_surface_format.colorSpace,
            .imageExtent = m_swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };

        auto indices = m_physical.get_queue_indices();
        std::array<uint32_t, 2> queue_family_indices = {indices.Graphics, m_presentation_index};

        if(indices.Graphics != m_presentation_index){
            swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_ci.queueFamilyIndexCount = 2;
            swapchain_ci.pQueueFamilyIndices = queue_family_indices.data();
        }
        else{
            swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchain_ci.preTransform = m_surface_capabilities.currentTransform;
        swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_ci.presentMode = m_presentation_mode;
        swapchain_ci.clipped = VK_TRUE;

        auto swapchain_create_res = vkCreateSwapchainKHR(m_driver, &swapchain_ci, nullptr, &m_swapchain_handler);

        if(swapchain_create_res != VK_SUCCESS){
            fmt::println("Swapchain VkResult Failed returning status = {}", (int)swapchain_create_res);
            return;
        }

        m_swapchain_image_format = m_current_selected_surface_format.format;
        fmt::println("Swapchain End Successful Initialization!!!");

        on_create_images();
        on_create_swapchain_renderpass();

        on_create_swapchain_framebuffers();
    }

    void swapchain::on_create_swapchain_framebuffers(){

        fmt::println("\nVulkan Swapchain Begin framebuffer initialization!!!");
        m_swapchain_framebuffers.resize(m_swapchain_images.size());

        for(uint32_t i = 0; i < m_swapchain_framebuffers.size(); i++){
            VkImageView image_view = m_swapchain_images[i].ImageView;

            VkFramebufferCreateInfo framebuffer_ci = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_swapchain_renderpass,
                .attachmentCount = 1,
                .pAttachments = &image_view,
                .width = m_swapchain_extent.width,
                .height = m_swapchain_extent.height,
                .layers = 1,
            };

            vk_check(vkCreateFramebuffer(m_driver, &framebuffer_ci, nullptr, &m_swapchain_framebuffers[i]), "vkCreateImageView", __FILE__, __LINE__, __FUNCTION__);
        }

        fmt::println("Vulkan Swapchain End framebuffer initialization successfully!!!!!\n");
    }

    void swapchain::on_create_swapchain_renderpass(){
        fmt::println("\nVulkan Swapchain Begin Render Pass Initialization!!!");

        VkAttachmentDescription colorAttachment{
            .format = m_swapchain_image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderpass_ci = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency,
        };

        vk_check(vkCreateRenderPass(m_driver, &renderpass_ci, nullptr, &m_swapchain_renderpass), "vkCreateRenderPass", __FILE__, __LINE__, __FUNCTION__);

        fmt::println("Vulkan Swapchain End RenderPass Successfully Initialization!!!\n");
    }


    void swapchain::on_create_images(){
        uint32_t image_count = -1;
        fmt::println("\nSwapchain Begin Image Initialization!");
        std::vector<VkImage> images;
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, nullptr);
        images.reserve(image_count);
        m_swapchain_images.reserve(image_count);

        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, images.data());

        for(uint32_t i = 0; i < m_swapchain_images.size(); i++){
            m_swapchain_images[i].Image = images[i];

            VkImageViewCreateInfo image_view_ci = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = m_swapchain_images[i].Image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_current_selected_surface_format.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_R,
                    .g = VK_COMPONENT_SWIZZLE_G,
                    .b = VK_COMPONENT_SWIZZLE_B
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            vk_check(vkCreateImageView(m_driver, &image_view_ci, nullptr, &m_swapchain_images[i].ImageView), "vkCreateImageView", __FILE__, __LINE__, __FUNCTION__);
        }

        fmt::println("Swapchain End Image Initialization Successfullyy!");
    }

    //! @note This might need to be uncommented because I realized that I have to VkSurfaceFormatKHR variables and they might do the same thing
    //! @note I need to compare the two afterwards the swapchain implementations are done

    void swapchain::on_swapchain_format_query(const VkPhysicalDevice& p_physical){
        fmt::println("Vulkan Swapchain Begin Quering Surface Formats!!");

        uint32_t format_count;
        std::vector<VkSurfaceFormatKHR> available_formats;
        //! @note Extracting the surface format that is supported on our current GPU (physical device)
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical, m_surface_handler, &format_count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FILE__, __LINE__, __FUNCTION__);

        available_formats.resize(format_count);
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical, m_surface_handler, &format_count, available_formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FILE__, __LINE__, __FUNCTION__);

        bool available_format_found = false;

        for (const auto& availableFormat : available_formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                // return availableFormat;
                available_format_found = true;
                m_current_selected_surface_format = availableFormat;
            }
        }

        //! @note If not available formats are found that we are searching for, then we assign the first format that we will use
        if(!available_format_found){
            m_current_selected_surface_format = available_formats[0];
        }

        fmt::println("Vulkan Swapchain Ended Quering Surface Formats!!");
    }

    void swapchain::on_swapchain_presentation_query(){
        fmt::println("Vulkan Swapchain Begin Quering Presentation Queue!!");

        uint32_t present_mode_count = -1;
        std::vector<VkPresentModeKHR> presentations;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_surface_handler, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            presentations.reserve(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_surface_handler, &present_mode_count, presentations.data());
        }

        bool is_presentation_mode_available = false;

        for(const auto& available_present_mode : presentations){
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                is_presentation_mode_available = true;
                m_presentation_mode = available_present_mode;
            }
        }

        if(!is_presentation_mode_available){
            fmt::println("\nVulkan Swapchain Presentation is VK_PRESENT_MODE_FIFO_KHR because\nno other queried modes were available!\n");
            m_presentation_mode = VK_PRESENT_MODE_FIFO_KHR;
        }

        fmt::println("Vulkan Swapchain Ended Quering Presentation Queue!!");
    }
};