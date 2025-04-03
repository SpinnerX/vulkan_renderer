#pragma once
#include <string>
#include <vulkan-cpp/vk_buffer.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_command_buffer.hpp>

namespace vk {
    /*
        Texture Mapping in Vulkan

        1. Load image from file
        2. Create texture object
        3. Allocate memory for texture
        4. Bind memory to texture object
        5. Upload image pixels to texture object
        6. Create Image View
        7. Create Sampler Object
        8. Add binding point to descriptor set
        9. Update descriptor set with handles of image view and sampler
        10. Add texture coordinates to SSBO ("vertex buffer")
        11. Pull texture coordinates from SSBO in Vertex Shader
        12. Pass texture cooridinates from VS to FS
        13. Sample form texture!!! (Final Step!!)

        ---------------------------------------
        1. Creating separate command buffer we are recording for this specific
       texture
        2. Then we submit this command buffer to the swapchain's main queue
    */

    /*

    Createing a texture
    1. Load Image Object
    2. Create staging buffer
    3. Transition Image Layout
    4. CopyBufferToImage
    5. TransitionImageLayout
    6. DestroyStagingBuffer


    These are the stpes in the order they are called for vk_texture
        - In terms of loading a texture image

    Loading image from stbi_load
        - load in image data using stbi_load API
        - load_image_view
        - load_sampler
        - load_texture_data_from_image

    1. load_texture_data_from_image
        * create_image
        * update_texture_image

    2. update_texture_image
        * creating staging buffer
        * map buffer
        * transition_img_layout
        * copy_buffer_to_image_layout
        * transition_image_layout
        * destroy staging buffer

    3. transition_image_layout
        * begin_command_bufer
        * image_memory_barrier
        * submit_copy_command_buffer

    4. copy_buffer_to_image
        * begin command buffer
        * vkCmdCopyBufferToImage
        * submit_copy_command_buffer

    5. submit_copy_command_buffer (just submits to the queue)
        - end_command_buffer
        - submit to queue
        - wait idle (or do VkFence stuff here)
        NOTE HERE: There was an error when I tried to learn how to get textures
    working, and this is because I was submitting to the wrong queue. Instead of
    submitting to presentation queue, you submit through the graphics queue
                   - Now that we know, I can cleanup the code to make more sense
    now!
    */
    class vk_texture {
    public:
        vk_texture() = default;
        //! @note Just so we can automatically submit to this queue to the GPU
        //! TODO: NEED to do a better way of doing this.
        vk_texture(const std::string& p_filename);

        /*

            1. CreateImage
            2. Update TextureImage
        */
        void create_texture_from_data(uint32_t p_width,
                                      uint32_t p_height,
                                      const void* p_pixels,
                                      const VkFormat p_format);

        // void update_texture(const VkCommandBuffer& p_command_buffer,
        // image_data& p_image_data, uint32_t p_width, uint32_t p_height,
        // VkFormat p_format, const void* p_pixels);

        void update_texture(image_data& p_image_data,
                            uint32_t p_width,
                            uint32_t p_height,
                            VkFormat p_format,
                            const void* p_pixels);

        // functions used by update texture

        // functions for transition image layout
        void transition_image_layout(VkImage& p_image,
                                     VkFormat p_format,
                                     VkImageLayout p_old,
                                     VkImageLayout p_new);
        void image_memory_barrier(VkCommandBuffer& p_command_buffer,
                                  VkImage& p_image,
                                  VkFormat p_format,
                                  VkImageLayout p_old,
                                  VkImageLayout p_new);

        // functions for copy buffer to image
        void copy_buffer_to_image(VkImage& p_image,
                                  VkBuffer& p_buffer,
                                  uint32_t p_width,
                                  uint32_t p_height);

        image_data data() const { return m_texture_image; }

        void destroy();

        vk_command_buffer* get_command_buffer() {
            return &m_copy_command_buffer;
        }

        VkImageView image_view() const { return m_texture_image.ImageView; }

        VkSampler sampler() const { return m_texture_image.Sampler; }

    private:
        vk_driver m_driver;
        buffer_properties m_staging_buffer;
        image_data m_texture_image;
        vk_command_buffer m_copy_command_buffer;
        VkQueue m_graphics_queue = nullptr;
    };
};