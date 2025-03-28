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

    static VkImageView create_image_view(const VkDevice& p_driver, VkImage p_image, VkSurfaceFormatKHR p_surface_format, VkImageAspectFlags p_aspect_flags, VkImageViewType p_view_t, uint32_t p_layer_count, uint32_t p_mip_levels) {
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = p_image,
            .viewType = p_view_t,
            .format = p_surface_format.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = p_aspect_flags,
                .baseMipLevel = 0,
                .levelCount = p_mip_levels,
                .baseArrayLayer = 0,
                .layerCount = p_layer_count
            },
        };

        VkImageView image_view;
        vk_check(vkCreateImageView(p_driver, &image_view_ci, nullptr, &image_view), "vkCreateImageView", __FUNCTION__);

        return image_view;
    }

    static VkRenderPass create_simple_renderpass(const VkDevice& p_driver, const VkSurfaceFormatKHR& p_surface_format) {
        VkAttachmentDescription attachment_description = {
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

        VkAttachmentReference attachment_ref = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL 
        };

        VkSubpassDescription subpass_description = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachment_ref,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        VkRenderPassCreateInfo renderpass_ci = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = &attachment_description,
            .subpassCount = 1,
            .pSubpasses = &subpass_description,
            .dependencyCount = 0,
            .pDependencies = nullptr
        };

        VkRenderPass renderpass=nullptr;

        vk_check(vkCreateRenderPass(p_driver, &renderpass_ci, nullptr, &renderpass), "vkCreateRenderPass", __FUNCTION__);

        return renderpass;
    }

    vk_swapchain::vk_swapchain(vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface) : m_driver(p_driver), m_physical(p_physical), m_current_surface(p_surface) {
        m_surface_data = p_physical.get_surface_properties(p_surface);
        on_create();
    }

    void vk_swapchain::on_create() {
        console_log_info("vk_swapchain() begin initialization!!!");
        m_swapchain_size = m_surface_data.SurfaceCapabilities.currentExtent;

        // request what our minimum image count is
        uint32_t request_min_image_count = select_images_size(m_surface_data.SurfaceCapabilities);

        // setting our presentation properties
        uint32_t present_index = m_physical.get_presentation_index(m_current_surface);
        console_log_trace("Presentation Index = {}", present_index);
        m_present_queue = m_driver.get_presentation_queue(present_index);

        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_current_surface,
            .minImageCount = request_min_image_count,
            .imageFormat = m_surface_data.SurfaceFormat.format,
            .imageColorSpace = m_surface_data.SurfaceFormat.colorSpace,
            // use physical device surface formats to getting the right formats in vulkan
            .imageExtent = m_swapchain_size,
            .imageArrayLayers = 1,
            .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &present_index,
            .preTransform = m_surface_data.SurfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
            .clipped = true
        };

        vk_check(vkCreateSwapchainKHR(m_driver, &swapchain_ci, nullptr, &m_swapchain_handler), "vkCreateSwapchainKHR", __FUNCTION__);

        // querying images we have

        uint32_t image_count=0;
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, nullptr); // used to get the amount of images
        std::vector<VkImage> images(image_count);
        vkGetSwapchainImagesKHR(m_driver, m_swapchain_handler, &image_count, images.data()); // used to store in the images

        // setting images
        m_swapchain_images.resize(image_count);
        console_log_trace("swapchain images.size() = {}", m_swapchain_images.size());

        uint32_t layer_count = 1;
        uint32_t mip_levels = 1;
        for(uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            m_swapchain_images[i].Image = images[i];
            m_swapchain_images[i].ImageView = create_image_view(m_driver, images[i], m_surface_data.SurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, layer_count, mip_levels);
        }


        // command pools
        console_log_info("vk_swapchain begin initializing command pool!!!!");
        VkCommandPoolCreateInfo command_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = present_index,
        };

        vk_check(vkCreateCommandPool(m_driver, &command_pool_ci, nullptr, &m_command_pool), "vkCreateCommandPool", __FUNCTION__);
        console_log_info("vk_swapchain successfully initialized command pool!!!!\n");

        // command buffers
        console_log_info("vk_swapchain begin initializing command buffers!!!!");

        m_swapchain_command_buffers.resize(image_count);
        console_log_trace("command buffers.size() = {}", m_swapchain_command_buffers.size());
        VkCommandBufferAllocateInfo command_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(m_swapchain_command_buffers.size())
        };

        vk_check(vkAllocateCommandBuffers(m_driver, &command_buffer_alloc_info, m_swapchain_command_buffers.data()), "vkAllocateCommandBuffers", __FUNCTION__);

        console_log_info("vk_swapchain successfully initialized command buffers!!!!\n");


        // We dont need to specify queue information. This should be provided to by the swapchain
        // The queue is provided within the swapchain during its initialization phase
        m_swapchain_queue = vk_queue(m_driver, m_swapchain_handler, m_present_queue);

        m_swapchain_renderpass = create_simple_renderpass(m_driver, m_surface_data.SurfaceFormat);

        // creating framebuffers
        m_swapchain_framebuffers.resize(m_swapchain_images.size());

        for(uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            VkFramebufferCreateInfo framebuffer_ci = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = m_swapchain_renderpass,
                .attachmentCount = 1,
                .pAttachments = &m_swapchain_images[i].ImageView,
                .width = m_swapchain_size.width,
                .height = m_swapchain_size.height,
                .layers = 1
            };

            vk_check(vkCreateFramebuffer(m_driver, &framebuffer_ci, nullptr, &m_swapchain_framebuffers[i]), "vkCreateFramebuffer", __FUNCTION__);
        }


        console_log_info("vk_swapchain() successfully initialized!!!\n\n");
    }

    void vk_swapchain::recreate() {
        vkDeviceWaitIdle(m_driver);
        on_create();
    }

    void vk_swapchain::destroy() {

        // needed to be called to ensure all children objects are executed just before they get destroyed!!
        // vkDeviceWaitIdle(m_driver);

        for(size_t i = 0; i < m_swapchain_framebuffers.size(); i++) {
            vkDestroyFramebuffer(m_driver, m_swapchain_framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(m_driver, m_swapchain_renderpass, nullptr);

        m_swapchain_queue.destroy();

        vkDestroyCommandPool(m_driver, m_command_pool, nullptr);

        for(uint32_t i = 0; i < m_swapchain_images.size(); i++) {
            vkDestroyImageView(m_driver, m_swapchain_images[i].ImageView, nullptr);
        }

        vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
    }

    uint32_t vk_swapchain::read_acquired_image() {
        uint32_t image_index = 0;

        // vkAcquireNextImageKHR(m_driver, m_swapchain_handler, std::numeric_limits<uint32_t>::max(), m_swapchain_images_available[m_current_frame], VK_NULL_HANDLE, &image_index);
        m_current_image_index = image_index;
        return image_index;
    }

    void vk_swapchain::resize(uint32_t p_width, uint32_t p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }


};