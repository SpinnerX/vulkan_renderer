#include <vulkan-cpp/vk_texture.hpp>
#include <vulkan-cpp/logger.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan/vulkan.h>

#include <vulkan-cpp/vk_swapchain.hpp>

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

    VkSampler create_sampler(VkFilter MinFilter, VkFilter MaxFilter, VkSamplerAddressMode AddressMode){
        VkDevice driver = vk_driver::driver_context();

        VkSamplerCreateInfo SamplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = MinFilter,
            .minFilter = MaxFilter,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = AddressMode,
            .addressModeV = AddressMode,
            .addressModeW = AddressMode,
            .mipLodBias = 0.0f,
            .anisotropyEnable = false,
            .maxAnisotropy = 1,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false
        };

        VkSampler Sampler;
        VkResult res = vkCreateSampler(driver, &SamplerInfo, VK_NULL_HANDLE, &Sampler);
        vk_check(res, "vkCreateSampler", __FUNCTION__);

        return Sampler;
    }

    VkImageView create_image_view(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags){
        VkDevice driver = vk_driver::driver_context();

        VkImageViewCreateInfo ViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = Image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = Format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = AspectFlags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
    };

        VkImageView ImageView;
        VkResult res = vkCreateImageView(driver, &ViewInfo, NULL, &ImageView);
        vk_check(res, "vkCreateImageView", __FUNCTION__);
        return ImageView;
        }


    image_data create_image2d(uint32_t p_width, uint32_t p_height, VkFormat p_format, VkImageUsageFlags p_usage, VkMemoryPropertyFlagBits p_property) {
        vk_driver driver = vk_driver::driver_context();

        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = p_format,
            .extent = {.width = p_width, .height = p_height, .depth = 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = p_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        // VkImage image=nullptr;
        image_data image;
        image.Width = p_width;
        image.Height = p_height;

        vk_check(vkCreateImage(driver, &image_ci, nullptr, &image.Image), "vkCreateImage", __FUNCTION__);

        // 2. get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(driver, image.Image, &memory_requirements);

        // 3. get memory type index
        uint32_t memory_type_index = driver.select_memory_type(memory_requirements.memoryTypeBits, p_property);

        // 4. Allocate info
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(vkAllocateMemory(driver, &memory_alloc_info, nullptr, &image.DeviceMemory), "vkAllocateMemory", __FUNCTION__);

        // 5. bind image memory
        vk_check(vkBindImageMemory(driver, image.Image, image.DeviceMemory, 0), "vkBindImageMemory", __FUNCTION__);

        return image;
    }

    bool has_stencil_attachment(VkFormat p_format) {
        return ((p_format == VK_FORMAT_D32_SFLOAT_S8_UINT) || 
		    (p_format == VK_FORMAT_D24_UNORM_S8_UINT));
    }

    vk_texture::vk_texture(const std::string& p_filename){
        console_log_info("vk_texture begin initialization!!!");

        /**
            TODO: Refactor vk_texture
                - probably have functions that already pre-load images for you
                - have functions for loading in samplers as well or entire abstractions for the way vulkan does it
        */

        m_driver = vk_driver::driver_context();
        m_graphics_queue = m_driver.get_graphics_queue();

        command_buffer_properties properties = {
            0,
            command_buffer_levels::Primary,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };

        m_copy_command_buffer = vk_command_buffer(properties);

        if(m_copy_command_buffer == nullptr) {
            console_log_error("m_copy_command_buffer IS NULL!!!");
        }
        else {
            console_log_info("m_copy_command_buffer NOT NULL!!!");
        }

        int w, h;
        int channels;

        // 1. load from file
        stbi_uc* image_data = stbi_load(p_filename.c_str(), &w, &h, &channels, STBI_rgb_alpha);
        VkDeviceSize image_size = w * h * 4;

        if(!image_data) {
            console_log_warn("Could not load filename with = {}", p_filename);
            return;
        }
        else {
            console_log_trace("Loaded {} successfully!!!", p_filename);
        }

        // image_data
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

        // 1. creating image data
        // 2. updating texture data
        create_texture_from_data(w, h, image_data, format);

        stbi_image_free(image_data);

        // 3. create image view
        VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        m_texture_image.ImageView = create_image_view(m_texture_image.Image, format, aspect_flags);

        VkFilter min_filter = VK_FILTER_LINEAR;
        VkFilter max_filter = VK_FILTER_LINEAR;
        VkSamplerAddressMode addr_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        m_texture_image.Sampler = create_sampler(min_filter, max_filter, addr_mode);


        console_log_info("vk_texture begin successful initialization!!!");
    }
    

    void vk_texture::create_texture_from_data(uint32_t p_width, uint32_t p_height, const void* p_pixels, VkFormat p_format) {
        console_log_info("create_texture_from_data begin initialization!!!");
        VkImageUsageFlagBits usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        VkMemoryPropertyFlagBits property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // 1. create image  object
        m_texture_image = create_image2d(p_width, p_height, p_format, usage, property);

        // 2. update texture data
        update_texture(m_texture_image, p_width, p_height, p_format, p_pixels);

        console_log_fatal("create_texture_from_data update END initialization!!!\n\n");
    }

    // void vk_texture::update_texture(const VkCommandBuffer& p_command_buffer, image_data& p_image_data, uint32_t p_width, uint32_t p_height, VkFormat p_format, const void* p_pixels) {
    void vk_texture::update_texture(image_data& p_image_data, uint32_t p_width, uint32_t p_height, VkFormat p_format, const void* p_pixels) {


        // 1. bytes per pixels
        int bytes_per_pixels = bytes_per_texture_format(p_format);

        // 2. layer_size
        VkDeviceSize layer_size = p_width * p_height * bytes_per_pixels;
        int layer_count = 1;
        VkDeviceSize image_size = layer_count * layer_size;

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        // 3. creating data for staging buffers
        m_staging_buffer = create_buffer(image_size, usage, property);

        // 4. maps the buffer data to that parameters
        write(m_staging_buffer, p_pixels, image_size);

        // 5. transition image layout
        transition_image_layout(p_image_data.Image, p_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // // 6. Copy buffer to image
        copy_buffer_to_image(p_image_data.Image, m_staging_buffer.BufferHandler, p_width, p_height);

        // // 7. transition image layout again
        transition_image_layout(p_image_data.Image, p_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        console_log_error("create_texture_from_data end initialization successfully!!!");
    }

    // This should automatically submit to the swaphain to the swapchain queue-specifically
    void vk_texture::transition_image_layout(VkImage& p_image, VkFormat p_format, VkImageLayout p_old, VkImageLayout p_new) {
        m_copy_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // 1. image memory barrier
        VkCommandBuffer handle = m_copy_command_buffer.handle();
        image_memory_barrier(handle, p_image, p_format, p_old, p_new);

        // submit one copy
        m_copy_command_buffer.end();

        VkCommandBuffer buffer = m_copy_command_buffer.handle();

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &buffer,
        };

        vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphics_queue);

    }

    void vk_texture::image_memory_barrier(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkFormat p_format, VkImageLayout p_old, VkImageLayout p_new) {
        VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = p_old,
            .newLayout = p_new,
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

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags dst_stages;

        if (p_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            (p_format == VK_FORMAT_D16_UNORM) ||
            (p_format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
            (p_format == VK_FORMAT_D32_SFLOAT) ||
            (p_format == VK_FORMAT_S8_UINT) ||
            (p_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
            (p_format == VK_FORMAT_D24_UNORM_S8_UINT)){
            image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if(has_stencil_attachment(p_format)) {
                image_memory_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (p_old == VK_IMAGE_LAYOUT_UNDEFINED && p_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (p_old == VK_IMAGE_LAYOUT_UNDEFINED && p_new == VK_IMAGE_LAYOUT_GENERAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    
        if (p_old == VK_IMAGE_LAYOUT_UNDEFINED && 
            p_new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert back from read-only to updateable */
        else if (p_old == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
            source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (p_old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                 p_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert depth texture from undefined state to depth-stencil buffer */
        else if (p_old == VK_IMAGE_LAYOUT_UNDEFINED && p_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } /* Wait for render pass to complete */
        else if (p_old == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier.dstAccessMask = 0;
            /*
                    source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ///		dst_stages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    dst_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            */
            source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to color attachment */
        else if (p_old == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
            source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } /* Convert from updateable texture to shader read-only */
        else if (p_old == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } /* Convert back from read-only to depth attachment */
        else if (p_old == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
            source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } /* Convert from updateable depth texture to shader read-only */
        else if (p_old == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && p_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
            source_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(p_command_buffer, source_stage, dst_stages, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }

    void vk_texture::copy_buffer_to_image(VkImage& p_image, VkBuffer& p_buffer, uint32_t p_width, uint32_t p_height) {
        m_copy_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
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

        vkCmdCopyBufferToImage(m_copy_command_buffer, p_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

        m_copy_command_buffer.end();

        VkCommandBuffer buffer = m_copy_command_buffer.handle();

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &buffer,
        };

        vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphics_queue);
    }

    void vk_texture::destroy() {
        vkDestroyImageView(m_driver, m_texture_image.ImageView, nullptr);
        vkDestroyImage(m_driver, m_texture_image.Image, nullptr);
        vkDestroySampler(m_driver, m_texture_image.Sampler, nullptr);

        vkFreeMemory(m_driver, m_texture_image.DeviceMemory, nullptr);

        vkDestroyBuffer(m_driver, m_staging_buffer.BufferHandler, nullptr);
        vkFreeMemory(m_driver, m_staging_buffer.DeviceMemory, nullptr);

        m_copy_command_buffer.destroy();
    }

};