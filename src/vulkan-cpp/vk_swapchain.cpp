#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

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

    // validate the capabilities to ensure we are not requesting the maximum
    // over the amount of images we are able to request
    uint32_t select_images_size(
      const VkSurfaceCapabilitiesKHR& p_surface_capabilities) {
        uint32_t requested_images = p_surface_capabilities.minImageCount + 1;

        uint32_t final_image_count = 0;

        if ((p_surface_capabilities.maxImageCount > 0) and
            (requested_images > p_surface_capabilities.maxImageCount)) {
            final_image_count = p_surface_capabilities.maxImageCount;
        }
        else {
            final_image_count = requested_images;
        }

        return final_image_count;
    }

    static VkRenderPass create_simple_renderpass(
      const VkDevice& p_driver,
      const VkSurfaceFormatKHR& p_surface_format) {
        VkFormat depth_format = vk_driver::depth_format();
        VkAttachmentDescription color_attachment_description = {
            .flags = 0,
            .format = p_surface_format.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentDescription depth_attachment_description = {
            .flags = 0,
            .format = depth_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference color_attachment_ref = {
            .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depth_attachment_reference = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass_description = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment =
              &depth_attachment_reference, // enable depth buffering
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        std::vector<VkAttachmentDescription> attachments;
        attachments.push_back(color_attachment_description);
        attachments.push_back(depth_attachment_description);

        VkRenderPassCreateInfo renderpass_ci = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            // .attachmentCount = 1,
            // .pAttachments = &attachment_description,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass_description,
            .dependencyCount = 0,
            .pDependencies = nullptr
        };

        VkRenderPass renderpass = nullptr;

        vk_check(
          vkCreateRenderPass(p_driver, &renderpass_ci, nullptr, &renderpass),
          "vkCreateRenderPass",
          __FUNCTION__);

        return renderpass;
    }

    vk_swapchain::vk_swapchain(vk_physical_driver& p_physical,
                               const vk_driver& p_driver,
                               const VkSurfaceKHR& p_surface)
      : m_driver(p_driver)
      , m_physical(p_physical)
      , m_current_surface(p_surface) {
        m_surface_data = p_physical.get_surface_properties(p_surface);
        on_create();
    }

    void vk_swapchain::on_create() {
        console_log_info("vk_swapchain() begin initialization!!!");
        m_swapchain_size = m_surface_data.SurfaceCapabilities.currentExtent;

        // request what our minimum image count is
        uint32_t request_min_image_count =
          select_images_size(m_surface_data.SurfaceCapabilities);

        // setting our presentation properties
        uint32_t present_index =
          m_physical.get_presentation_index(m_current_surface);
        console_log_trace("Presentation Index = {}", present_index);
        m_present_queue = m_driver.get_presentation_queue(present_index);

        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_current_surface,
            .minImageCount = request_min_image_count,
            .imageFormat = m_surface_data.SurfaceFormat.format,
            .imageColorSpace = m_surface_data.SurfaceFormat.colorSpace,
            // use physical device surface formats to getting the right formats
            // in vulkan
            .imageExtent = m_swapchain_size,
            .imageArrayLayers = 1,
            .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &present_index,
            .preTransform = m_surface_data.SurfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
            .clipped = true
        };

        vk_check(vkCreateSwapchainKHR(
                   m_driver, &swapchain_ci, nullptr, &m_swapchain_handler),
                 "vkCreateSwapchainKHR",
                 __FUNCTION__);

        // querying images we have

        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(m_driver,
                                m_swapchain_handler,
                                &image_count,
                                nullptr); // used to get the amount of images
        std::vector<VkImage> images(image_count);
        vkGetSwapchainImagesKHR(m_driver,
                                m_swapchain_handler,
                                &image_count,
                                images.data()); // used to store in the images

        // Creating Images
        m_swapchain_images.resize(image_count);
        m_swapchain_depth_images.resize(image_count);

        console_log_trace("swapchain images.size() = {}",
                          m_swapchain_images.size());
        VkFormat depth_format = m_driver.depth_format();
        uint32_t layer_count = 1;
        uint32_t mip_levels = 1;
        for (uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            m_swapchain_images[i].Image = images[i];
            m_swapchain_images[i].ImageView =
              create_image_view(m_driver,
                                images[i],
                                m_surface_data.SurfaceFormat,
                                VK_IMAGE_ASPECT_COLOR_BIT,
                                VK_IMAGE_VIEW_TYPE_2D,
                                layer_count,
                                mip_levels);

            // Creating Depth Images for depth buffering
            VkImageUsageFlagBits usage =
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            VkMemoryPropertyFlagBits property_flags =
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            // Creates our images
            m_swapchain_depth_images[i] = create_image2d(m_swapchain_size.width,
                                                       m_swapchain_size.height,
                                                       depth_format,
                                                       usage,
                                                       property_flags);
            m_swapchain_depth_images[i].ImageView =
              create_image_view(m_swapchain_depth_images[i].Image,
                                depth_format,
                                VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        // command buffers
        console_log_info("vk_swapchain begin initializing command buffers!!!!");

        m_swapchain_command_buffers.resize(image_count);
        console_log_trace("command buffers.size() = {}",
                          m_swapchain_command_buffers.size());

        for (size_t i = 0; i < m_swapchain_command_buffers.size(); i++) {
            command_buffer_properties properties = {
                present_index,
                command_buffer_levels::Primary,
                (VkCommandPoolCreateFlagBits)0
            };

            m_swapchain_command_buffers[i] = vk_command_buffer(properties);
        }

        console_log_info(
          "vk_swapchain successfully initialized command buffers!!!!\n");

        // We dont need to specify queue information. This should be provided to
        // by the swapchain The queue is provided within the swapchain during
        // its initialization phase
        m_swapchain_present_queue =
          vk_queue(m_driver, m_swapchain_handler, m_present_queue);

        m_swapchain_renderpass =
          create_simple_renderpass(m_driver, m_surface_data.SurfaceFormat);

        // creating framebuffers
        m_swapchain_framebuffers.resize(m_swapchain_images.size());

        for (uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            std::vector<VkImageView> image_view_attachments;
            image_view_attachments.push_back(m_swapchain_images[i].ImageView);
            image_view_attachments.push_back(
              m_swapchain_depth_images[i].ImageView);

            VkFramebufferCreateInfo framebuffer_ci = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = m_swapchain_renderpass,
                // .attachmentCount = 1,
                // .pAttachments = &m_swapchain_images[i].ImageView,
                .attachmentCount =
                  static_cast<uint32_t>(image_view_attachments.size()),
                .pAttachments = image_view_attachments.data(),
                .width = m_swapchain_size.width,
                .height = m_swapchain_size.height,
                .layers = 1
            };

            vk_check(vkCreateFramebuffer(m_driver,
                                         &framebuffer_ci,
                                         nullptr,
                                         &m_swapchain_framebuffers[i]),
                     "vkCreateFramebuffer",
                     __FUNCTION__);
        }

        console_log_info("vk_swapchain() successfully initialized!!!\n\n");
    }

    void vk_swapchain::recreate() {
        vkDeviceWaitIdle(m_driver);
        on_create();
    }

	uint32_t vk_swapchain::read_acquired_frame() {
		//! @note We always want to wait until the current frame is ready before moving onto the next frame
		m_swapchain_present_queue.wait_idle();
		uint32_t current_frame = m_swapchain_present_queue.read_acquire_image();
		m_current_image_index = current_frame;
		return current_frame;
	}

	void vk_swapchain::submit(const VkCommandBuffer& p_current) {
		m_swapchain_present_queue.submit_to(p_current,submission_type::Async);
	}

	void vk_swapchain::present() {

		m_swapchain_present_queue.present(m_current_image_index);
	}

    void vk_swapchain::destroy() {

        // needed to be called to ensure all children objects are executed just
        // before they get destroyed!! vkDeviceWaitIdle(m_driver);

        for (size_t i = 0; i < m_swapchain_framebuffers.size(); i++) {
            vkDestroyFramebuffer(
              m_driver, m_swapchain_framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(m_driver, m_swapchain_renderpass, nullptr);

        m_swapchain_present_queue.destroy();

        // vkDestroyCommandPool(m_driver, m_command_pool, nullptr);
        for (size_t i = 0; i < m_swapchain_command_buffers.size(); i++) {
            m_swapchain_command_buffers[i].destroy();
        }

        for (uint32_t i = 0; i < m_swapchain_depth_images.size(); i++) {
            vkDestroyImageView(
              m_driver, m_swapchain_depth_images[i].ImageView, nullptr);
            vkDestroyImage(
              m_driver, m_swapchain_depth_images[i].Image, nullptr);
            vkFreeMemory(
              m_driver, m_swapchain_depth_images[i].DeviceMemory, nullptr);
        }

        for (uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            vkDestroyImageView(
              m_driver, m_swapchain_images[i].ImageView, nullptr);
        }

        vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
    }

    void vk_swapchain::resize(uint32_t p_width, uint32_t p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }

};