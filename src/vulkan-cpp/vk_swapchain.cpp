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

    vk_swapchain::vk_swapchain(const vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface) : m_physical(p_physical), m_driver(p_driver), m_current_surface(p_surface) {
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
        m_current_frame = 0;


    }

    vk_swapchain::~vk_swapchain() {

        console_log_trace("begin ~vk_swapchain cleanup destructor!!!");
        for(size_t i = 0; i < swapchain_configs::MaxFramesInFlight; i++){
            vkDestroySemaphore(m_driver, m_swapchain_images_available[i], nullptr);
            vkDestroySemaphore(m_driver, m_swapchain_rendered_images_completed[i], nullptr);
            vkDestroyFence(m_driver, m_swapchain_fences_in_flight[i], nullptr);
        }

        vkDestroyRenderPass(m_driver, m_renderpass, nullptr);

        for(const VkFramebuffer& fb : m_swapchain_framebuffers){
            vkDestroyFramebuffer(m_driver, fb, nullptr);
        }

        for(const image& each_image : m_swapchain_images){
            vkDestroyImageView(m_driver, each_image.ImageView, nullptr);
        }

        if(m_swapchain_handler != nullptr) {
            vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
        }

        console_log_trace("finished ~vk_swapchain cleanup destructor!!!");
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

        create_semaphores();
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
        uint32_t image_count;
        console_log_info("\nSwapchain Begin Image Initialization!");
        std::vector<VkImage> images;
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, nullptr);
        images.resize(image_count);
        // m_swapchain_images.resize(image_count);
        console_log_trace("image_count = {}", image_count);
        console_log_trace("images.size() = {}", images.size());

        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, images.data());

        m_swapchain_fences_in_flight.resize(image_count);

        for(uint32_t i = 0; i < images.size(); i++){
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

    void vk_swapchain::create_semaphores() {
        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        for(size_t i = 0; i < m_swapchain_images_available.size(); i++){
            vk_check(vkCreateSemaphore(m_driver, &semaphore_create_info, nullptr, &m_swapchain_images_available[i]), "vkCreateSemaphore", __FUNCTION__);
            vk_check(vkCreateSemaphore(m_driver, &semaphore_create_info, nullptr, &m_swapchain_rendered_images_completed[i]), "vkCreateSemaphore", __FUNCTION__);

            vk_check(vkCreateFence(m_driver, &fence_create_info, nullptr, &m_swapchain_fences_in_flight[i]), "vkCreateFence", __FUNCTION__);
        }
    }

    void vk_swapchain::create_renderpass_for_swapchain() {
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





    uint32_t vk_swapchain::read_acquired_image() {
        vkWaitForFences(m_driver, 1, &m_swapchain_fences_in_flight[m_current_frame], true, std::numeric_limits<uint32_t>::max());
        uint32_t image_index;

        vkAcquireNextImageKHR(m_driver, m_swapchain_handler, std::numeric_limits<uint32_t>::max(), m_swapchain_images_available[m_current_frame], VK_NULL_HANDLE, &image_index);
        m_current_image_index = image_index;
        return image_index;
    }

    void vk_swapchain::submit_to(const VkCommandBuffer& p_current_command_buffer) {
        
        // if(m_swapchain_fences_in_flight[m_current_frame] != VK_NULL_HANDLE) {
        vkWaitForFences(m_driver, 1, &m_swapchain_fences_in_flight[m_current_frame], true, std::numeric_limits<uint32_t>::max());
        vkQueueWaitIdle(m_driver.get_graphics_queue());
        vkResetFences(m_driver, 1, &m_swapchain_fences_in_flight[m_current_frame]);

        VkSemaphore waitSemaphores[] = {m_swapchain_images_available[m_current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {m_swapchain_rendered_images_completed[m_current_frame]};

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &p_current_command_buffer,

            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
        };

        // if (vkQueueSubmit(m_driver.get_graphics_queue(), 1, &submitInfo, m_swapchain_fences_in_flight[m_current_frame]) != VK_SUCCESS) {
        //     throw std::runtime_error("failed to submit draw command buffer!");
        // }
        vk_check(vkQueueSubmit(m_driver.get_graphics_queue(), 1, &submitInfo, m_swapchain_fences_in_flight[m_current_frame]), "vkQueueSubmit", __FUNCTION__);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapchain_handler};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &m_current_image_index;

        vk_check(vkQueuePresentKHR(m_present_queue, &presentInfo), "vkQueuePresentKHR", __FUNCTION__);

        m_current_frame = (m_current_frame + 1) % swapchain_configs::MaxFramesInFlight;
    }




    /*
    uint32_t vk_swapchain::read_acquired_image() {
        uint32_t image_index;
        vk_check(
          vkWaitForFences(m_driver,
                          1,
                          &m_swapchain_fences_in_flight[FrameIndex],
                          true,
                          std::numeric_limits<uint32_t>::max()),
          "vkWaitForFences",
          __FUNCTION__);

        VkResult res = vkAcquireNextImageKHR(
          m_driver,
          m_swapchain_handler,
          std::numeric_limits<uint64_t>::max(),
          m_swapchain_images_available[FrameIndex],
          VK_NULL_HANDLE,
          &image_index);
        
          if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            return -3;
        }

        vk_check(res, "vkAcquireNextImageKHR", __FUNCTION__);

        // FrameIndex = image_index;
        ImageIndex = image_index;
        return image_index;
    }

    void vk_swapchain::submit_to(const VkCommandBuffer& p_current_command_buffer) {
        if(m_swapchain_fences_images_available[ImageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_driver, 1, &m_swapchain_fences_in_flight[ImageIndex], true, std::numeric_limits<uint32_t>::max());
        }

        m_swapchain_fences_images_available[ImageIndex] = m_swapchain_fences_in_flight[FrameIndex];

        VkSubmitInfo submission_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
        };
        
        VkSemaphore wait_semaphore[] = {
            m_swapchain_images_available[FrameIndex]
        };

        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        submission_info.waitSemaphoreCount = 1;
        submission_info.pWaitSemaphores = wait_semaphore;
        submission_info.pWaitDstStageMask = wait_stages;

        submission_info.commandBufferCount = 1;
        submission_info.pCommandBuffers = &p_current_command_buffer;

        VkSemaphore signal_sems[] = {
            m_swapchain_rendered_images_completed[FrameIndex]
        };

        submission_info.signalSemaphoreCount = 1;
        submission_info.pSignalSemaphores = signal_sems;

        vkResetFences(m_driver, 1, &m_swapchain_fences_in_flight[FrameIndex]);

        vk_check(vkQueueSubmit(m_driver.get_graphics_queue(), 1, &submission_info, m_swapchain_fences_in_flight[FrameIndex]), "vkQueueSubmit", __FUNCTION__);

        //! @note Now we present our data to the display.
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_sems,
        };

        VkSwapchainKHR swapchains_array[] = { m_swapchain_handler };
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains_array;

        present_info.pImageIndices = &ImageIndex;
        if (m_present_queue == VK_NULL_HANDLE) {
            console_log_error("PresentationQueue is nullptr!!!");
        }

        auto res = vkQueuePresentKHR(m_present_queue, &present_info);
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            console_log_trace("VK_ERROR_OUT_OF_DATE_KHR occurrred!!!");
            return;
        }

        vk_check(res, "vkQueuePresentKHR", __FUNCTION__);

        FrameIndex = (FrameIndex + 1) % swapchain_configs::MaxFramesInFlight;

        console_log_fatal("FrameIndex = {}", FrameIndex);

    }
    */

    void vk_swapchain::resize(uint32_t p_width, uint32_t p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }

};