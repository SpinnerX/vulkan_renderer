#include <vulkan-cpp/vk_texture.hpp>
#include <vulkan-cpp/logger.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan/vulkan.h>

namespace vk {

    int bytes_per_texture_format(VkFormat p_format) {
        switch (p_format){
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_UNORM: return 1;
        case VK_FORMAT_R16_SFLOAT: return 2;
        case VK_FORMAT_R16G16_SFLOAT: return 4;
        case VK_FORMAT_B8G8R8A8_UNORM: return 4;
        case VK_FORMAT_R8G8B8A8_UNORM: return 4;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return 4 * sizeof(uint16_t);
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 4 * sizeof(float);
        default:
            console_log_fatal("Error unknown format!!!");
            return 0;
        }

        return 0;
    }

    texture_properties create_image(const void* p_pixels, uint32_t p_width, uint32_t p_height, VkImageUsageFlags p_usage, VkMemoryPropertyFlagBits p_property_flags) {
        vk_driver driver = vk_driver::driver_context();

        texture_properties return_properties{};

        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent = {.width = p_width, .height = p_height, .depth = 1},
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = p_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        // 1. Create Image Object
        // VkImage image=nullptr;
        vk_check(vkCreateImage(driver, &image_ci, nullptr, &return_properties.Image), "vkCreateImage", __FUNCTION__);

        // 2. get IMAGE buffer memory requirements
        VkMemoryRequirements memory_requirements = {0};
        vkGetImageMemoryRequirements(driver, return_properties.Image, &memory_requirements);

        // 3. Get memory type index
        uint32_t memory_type_index = driver.select_memory_type(memory_requirements.memoryTypeBits, p_property_flags);

        // 4. Allocate memory based on memory requirements (to VkDeviceMemory)

        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,.
            allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(vkAllocateMemory(driver, &memory_alloc_info, nullptr, &return_properties.DeviceMemory), "vkAllocateMemory", __FUNCTION__);

        return return_properties;
    }

    bool has_stencil_attachment(VkFormat p_format) {
        return ((p_format == VK_FORMAT_D32_SFLOAT_S8_UINT) || 
		    (p_format == VK_FORMAT_D24_UNORM_S8_UINT));
    }

    void image_memory_barrier(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkFormat p_format, VkImageLayout p_old_layout, VkImageLayout p_new_layout) {
        VkImageMemoryBarrier image_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = p_old_layout,
            .newLayout = p_new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = p_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkPipelineStageFlags source_stages;
        VkPipelineStageFlags dst_stages;

        // if(p_new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        if (p_new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            (p_format == VK_FORMAT_D16_UNORM) ||
            (p_format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
            (p_format == VK_FORMAT_D32_SFLOAT) ||
            (p_format == VK_FORMAT_S8_UINT) ||
            (p_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
            (p_format == VK_FORMAT_D24_UNORM_S8_UINT)){
            image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if(has_stencil_attachment(p_format)) {
                image_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED && p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED && p_new_layout == VK_IMAGE_LAYOUT_GENERAL) {
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    
        if (p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED && 
            p_new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
            source_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert back from read-only to updateable */
        else if (p_old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
            source_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (p_old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                 p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert depth texture from undefined state to depth-stencil buffer */
        else if (p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED && p_new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
            source_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } /* Wait for render pass to complete */
        else if (p_old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
            image_barrier.dstAccessMask = 0;
            /*
                    source_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ///		dst_stages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    dst_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            */
            source_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to color attachment */
        else if (p_old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
            source_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (p_old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to depth attachment */
        else if (p_old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
            source_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } /* Convert from updateable depth texture to shader read-only */
        else if (p_old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(p_command_buffer, source_stages, dst_stages, 0, 0, nullptr, 0, nullptr, 1, &image_barrier);
    }

    VkImageView create_image_view(VkImage p_image, VkFormat p_format, VkImageAspectFlags p_aspect_flags) {
        VkDevice driver = vk_driver::driver_context();

        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = p_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = p_aspect_flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VkImageView image_view=nullptr;

        vk_check(vkCreateImageView(driver, &image_view_ci, nullptr, &image_view), "vkCreateImageView", __FUNCTION__);
        return image_view;
    }

    VkSampler create_sampler(VkFilter p_min_filter, VkFilter p_max_filter, VkSamplerAddressMode p_address_mode) {
        VkDevice driver = vk_driver::driver_context();
        VkSamplerCreateInfo sampler_ci = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = p_min_filter,
            .minFilter = p_max_filter,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = p_address_mode,
            .addressModeV = p_address_mode,
            .addressModeW = p_address_mode,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
    
        VkSampler sampler=nullptr;
        vk_check(vkCreateSampler(driver, &sampler_ci, nullptr, &sampler), "vkCreateSampler", __FUNCTION__);
        return nullptr;
    }

    vk_texture::vk_texture(const std::string& p_filename, vk_swapchain& p_current_swapchain) : m_copy_swapchain(p_current_swapchain){
        m_driver = vk_driver::driver_context();
        // 1. Create this first before submitting
        m_command_pool = create_single_command_pool();
        m_texture_copy_command_buffer = create_single_command_buffer(m_command_pool);

        int w, h;
        int channels;

        // 1. load from file
        stbi_uc* image_data = stbi_load(p_filename.c_str(), &w, &h, &channels, STBI_rgb_alpha);

        if(!image_data) {
            console_log_warn("Could not load filename with = {}", p_filename);
            return;
        }

        // 2. Create image object and populate with pixels
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        m_texture_properties = load_texture_image_from_data(image_data, w, h, format);

        // 3. Release image pixels
        stbi_image_free(image_data);

        // 4. create image view
        VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        m_texture_properties.ImageView = create_image_view(m_texture_properties.Image, format, aspect_flags);

        // 5. create texture sampler

        VkFilter min_filter = VK_FILTER_LINEAR;
        VkFilter max_filter = VK_FILTER_LINEAR;
        VkSamplerAddressMode sampler_addr_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        m_texture_properties.Sampler = create_sampler(min_filter, max_filter, sampler_addr_mode);
    }

    /*
    
        1. Creates VkImage (create_image function)
        2. Update Texture Image (update_texture_imaage function)
    */
    texture_properties vk_texture::load_texture_image_from_data(const void* p_pixels, uint32_t p_width, uint32_t p_height, VkFormat p_format) {
        
        texture_properties new_properties{};

        VkImageUsageFlagBits usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        VkMemoryPropertyFlagBits property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // 1. Create image object and set the properties of that
        new_properties = create_image(p_pixels, p_width, p_height, usage, property_flags);

        update_texture_image(new_properties, p_width, p_height, p_format, p_pixels);


        return new_properties;
    }


    void vk_texture::update_texture_image(texture_properties& p_properties, uint32_t p_width, uint32_t p_height, VkFormat p_format, const void* p_pixels) {
        
        // 1. get bytes per pixels
        int bytes_per_pixels = bytes_per_texture_format(p_format);

        VkDeviceSize layer_size = p_width * p_height * bytes_per_pixels;
        int layer_count = 1;
        VkDeviceSize image_size = layer_count * layer_size;

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        // 2. create staging buffer
        m_texture_staging_buffer_properties = create_buffer(image_size, usage, properties);

        // 3. map (write) the data to specific GPU memory locations
        // update
        write(m_texture_staging_buffer_properties, p_pixels, image_size);

        // 4. transition image layout
        transition_image_layout(m_texture_copy_command_buffer, p_properties.Image, p_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // 5. copy buffer to image
        copy_buffer_to_image(m_texture_copy_command_buffer, m_texture_properties.Image, m_texture_staging_buffer_properties.BufferHandler, p_width, p_height);

        // 6. transition iamge layout
        transition_image_layout(m_texture_copy_command_buffer, p_properties.Image, p_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // 7. destroying staging buffer
        destroy_staging_buffer();
    }

    //! @note VkCommandBuffer is supplied by the texture class itself
    void vk_texture::transition_image_layout(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkFormat p_format, VkImageLayout p_old_layout, VkImageLayout p_new_layout) {

        // 1. begin command buffer
        begin_command_buffer(p_command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // 2. image memory barrier
        image_memory_barrier(p_command_buffer, p_image, p_format, p_old_layout, p_new_layout);

        // 3. submit copy command buffer (this will be done outside of the vk_texture class through a getter function)
        //  - submit_copy command_buffer does the following two things
        /*
            3.1 VkEndCommandBuffer
            3.2 queue.submit_async
            3.3 queue.wait_idle()
        */
        // submit_copy_command
        submit_copy_command_buffer();
    }

    void vk_texture::copy_buffer_to_image(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkBuffer& p_buffer, uint32_t p_width, uint32_t p_height) {
        begin_command_buffer(m_texture_copy_command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkBufferImageCopy buffer_image_copy = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {.x = 0, .y = 0, .z = 0},
            .imageExtent = {.width = p_width, .height = p_height, .depth = 1}
        };

        vkCmdCopyBufferToImage(p_command_buffer, p_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

        submit_copy_command_buffer();
    }

    void vk_texture::submit_copy_command_buffer() {
        vkEndCommandBuffer(m_texture_copy_command_buffer);
        m_copy_swapchain.submit_buffer(m_texture_copy_command_buffer);
        // m_copy_queue.submit_to(m_texture_copy_command_buffer, submission_type::Async);
        // m_copy_queue.wait_idle();
    }
    
    void vk_texture::destroy() {
        vkDestroyImageView(m_driver, m_texture_properties.ImageView, nullptr);
        vkDestroyImage(m_driver, m_texture_properties.Image, nullptr);
        vkDestroySampler(m_driver, m_texture_properties.Sampler, nullptr);
        vkFreeMemory(m_driver, m_texture_properties.DeviceMemory, nullptr);
        vkDestroyCommandPool(m_driver, m_command_pool, nullptr);
    }

    void vk_texture::destroy_staging_buffer() {
        vkFreeMemory(m_driver, m_texture_staging_buffer_properties.DeviceMemory, nullptr);
        vkDestroyBuffer(m_driver, m_texture_staging_buffer_properties.BufferHandler, nullptr);
    }
};