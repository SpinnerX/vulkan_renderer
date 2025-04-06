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

        bool has_loaded() const { return m_is_image_loaded; }

        /**
         * @param p_width contains width of the image being loaded
         * @param p_height contains height of the image being loaded
         * @param p_pixel is the actual pixel data loaded from the filepath
         * @param p_format is the format the pixel data (image) is loaded with
        */
        void create_texture_from_data(uint32_t p_width,
                                      uint32_t p_height,
                                      const void* p_pixels,
                                      const VkFormat p_format);
        
        /**
         * @brief During updating this texture we do the following:
         * @brief Writing to a staging buffer
         * @brief then transition from the image we loaded and its layout
         * @brief copy from the buffer handler to a command buffer
         * @brief transitioning from that old image layout into a destination image layout specified
        */
        void update_texture(vk_image& p_vk_image,
                            uint32_t p_width,
                            uint32_t p_height,
                            VkFormat p_format,
                            const void* p_pixels);

        vk_image data() const { return m_texture_image; }

        //! @note Destroys the vulkan handlers cleanly
        //! @note TODO: This should probably be submitted into a deletion queue of some sort
        void destroy();

        VkImageView image_view() const { return m_texture_image.ImageView; }

        VkSampler sampler() const { return m_texture_image.Sampler; }

        uint32_t width() const { return m_width; }
        uint32_t height() const { return m_height; }

    private:
        vk_driver m_driver;
        buffer_properties m_staging_buffer;
        vk_image m_texture_image;
        vk_command_buffer m_copy_command_buffer;
        bool m_is_image_loaded=false;
        uint32_t m_width=0;
        uint32_t m_height=0;
    };
};