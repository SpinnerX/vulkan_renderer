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

    vk_swapchain::vk_swapchain(const vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface) : m_physical(p_physical), m_driver(p_physical), m_current_surface(p_surface) {
        //! @note 0.) Getting surface capabilities
        vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical, m_current_surface, &m_surface_capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",__FUNCTION__);

        //! @note 1.) Get Presentation Index and setting up presentation queue
        m_present_index = m_physical.get_presentation_index(p_surface);
        vkGetDeviceQueue(m_driver, m_present_index, 0, &m_present_queue);

        //! @note 2.) Get Presentation Mode
        //! @note Extracting number of presentation modes.
        select_presentation_mode();

        //! @note Getting surface size
        get_valid_extent();

        //! @note Getting swapchain on_create
        select_swapchain_surface_formats();


        on_create();

        s_instance = this;

    }

    vk_swapchain::~vk_swapchain() {

        // vkDestroyRenderPass(m_driver, m_renderpass, nullptr);

        for(const VkFramebuffer& fb : m_swapchain_framebuffers){
            vkDestroyFramebuffer(m_driver, fb, nullptr);
        }

        for(const image& each_image : m_swapchain_images){
            vkDestroyImageView(m_driver, each_image.ImageView, nullptr);
        }

        if(m_swapchain_handler != nullptr) {
            vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
        }

    }

    void vk_swapchain::on_create() {
        console_log_info("vk_swapchain::on_create begin initialization!!!");
        uint32_t image_count = m_surface_capabilities.minImageCount + 1;
        
        if(m_surface_capabilities.maxImageCount > 0 && image_count > m_surface_capabilities.maxImageCount){
            image_count = m_surface_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_current_surface,
            .minImageCount = image_count,
            .imageFormat = m_surface_format.format,
            .imageColorSpace = m_surface_format.colorSpace,
            .imageExtent = m_swapchain_size,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };

        auto indices = m_physical.get_queue_indices();
        std::array<uint32_t, 2> queue_family_indices = {indices.Graphics, m_present_index};

        if(indices.Graphics != m_present_index){
            swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_ci.queueFamilyIndexCount = 2;
            swapchain_ci.pQueueFamilyIndices = queue_family_indices.data();
        }
        else{
            swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchain_ci.preTransform = m_surface_capabilities.currentTransform;
        swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_ci.presentMode = m_present_mode;
        swapchain_ci.clipped = VK_TRUE;

        auto swapchain_create_res = vkCreateSwapchainKHR(m_driver, &swapchain_ci, nullptr, &m_swapchain_handler);

        if(swapchain_create_res != VK_SUCCESS){
            // fmt::println("Swapchain VkResult Failed returning status = {}", (int)swapchain_create_res);
            console_log_trace("Swapchain VkResult Failed returning status = {}", (int)swapchain_create_res);
            return;
        }

        console_log_info("vk_swapchain::on_create end initialization!!!\n\n");


        setup_images();
        create_renderpass_for_swapchain();

        create_swapchain_framebuffers();
    }

    void vk_swapchain::select_swapchain_surface_formats(){
        uint32_t format_count = 0;
        std::vector<VkSurfaceFormatKHR> formats;
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_current_surface, &format_count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FUNCTION__);
    
        formats.resize(format_count);

        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_current_surface, &format_count, formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FUNCTION__);

        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                m_surface_format = format;
            }
        }

        m_surface_format = formats[0];
    }

    void vk_swapchain::select_presentation_mode() {
        uint32_t preset_modes_count;
        std::vector<VkPresentModeKHR> presentation_modes;
        vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
                m_physical, m_current_surface, &preset_modes_count, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR",__FUNCTION__);
        presentation_modes.resize(preset_modes_count);
        vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_current_surface, &preset_modes_count, presentation_modes.data()), "vkGetPhysicalDeviceSurfacePresentModesKHR",__FUNCTION__);

        m_present_mode = select_compatible_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR, presentation_modes);
    }

    void vk_swapchain::get_valid_extent() {

        //! @note Width/Height of our current swapchain

        //! @note All this checks is the width/height are not set, then whatever
        //! the swapchain is set to will set the extent by default
        if(m_surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            m_swapchain_size = m_surface_capabilities.currentExtent;
        }

        m_swapchain_size = m_surface_capabilities.currentExtent;
    }



    void vk_swapchain::setup_images() {
        uint32_t image_count = -1;
        console_log_info("\nSwapchain Begin Image Initialization!");
        std::vector<VkImage> images;
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, nullptr);
        images.reserve(image_count);
        // m_swapchain_images.resize(image_count);

        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, images.data());

        for(uint32_t i = 0; i < m_swapchain_images.size(); i++){
            m_swapchain_images[i].Image = images[i];

            VkImageViewCreateInfo image_view_ci = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = m_swapchain_images[i].Image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_surface_format.format,
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

            vk_check(vkCreateImageView(m_driver, &image_view_ci, nullptr, &m_swapchain_images[i].ImageView), "vkCreateImageView", __FUNCTION__);
        }

        console_log_info("\nSwapchain Begin Image Initialization!!!\n\n");
    }

    void vk_swapchain::create_renderpass_for_swapchain() {

        // VkAttachmentDescription color_attachment = {
        //     .format = m_surface_format.format,
        //     .samples = VK_SAMPLE_COUNT_1_BIT,
        //     .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        //     .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        //     .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        //     .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        //     .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        // };

        // VkAttachmentReference color_attachment_reference = {
        //     .attachment = 0,
        //     .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        // };

        // VkSubpassDescription subpass = {
        //     .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        //     .colorAttachmentCount = 1,
        //     .pColorAttachments = &color_attachment_reference,
        // };

        // VkRenderPassCreateInfo renderpass_ci = {
        //     .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        //     .attachmentCount = 1,
        //     .pAttachments = &color_attachment,
        //     .subpassCount = 1,
        //     .pSubpasses = &subpass,
        // };
        console_log_trace("VkFormat = {}", vk_format_to_string(m_surface_format.format));
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_surface_format.format;
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


        vk_check(vkCreateRenderPass(m_driver, &renderPassInfo, nullptr, &m_renderpass), "vkCreateRenderPass", __FUNCTION__);
    }

    void vk_swapchain::create_swapchain_framebuffers() {
        for(size_t i = 0; i < m_swapchain_framebuffers.size(); i++){
            
            VkFramebufferCreateInfo fb_create_info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_renderpass,
                .attachmentCount = 1,
                .pAttachments = &m_swapchain_images[i].ImageView,
                .width = m_swapchain_size.width,
                .height = m_swapchain_size.height,
                .layers = 1,
            };

            vk_check(vkCreateFramebuffer(m_driver, &fb_create_info, nullptr, &m_swapchain_framebuffers[i]), "vkCreateFramebuffer", __FUNCTION__);
        }
    }

    void vk_swapchain::resize(uint32_t p_width, uint32_t p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }

};