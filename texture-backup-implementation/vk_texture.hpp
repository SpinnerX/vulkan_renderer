#pragma once
#include <string>
#include <vulkan-cpp/vk_buffer.hpp>
// #include <vulkan-cpp/vk_queue.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>

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
        1. Creating separate command buffer we are recording for this specific texture
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
    
    
    */
    class vk_texture {
    public:
        //! @note Just so we can automatically submit to this queue to the GPU
        //! TODO: NEED to do a better way of doing this.
        vk_texture(const std::string& p_filename, vk_swapchain& p_current_swapchain);

        void transition_image_layout(const VkCommandBuffer& p_command_buffer, VkImage& p_image, VkFormat p_format, VkImageLayout p_old, VkImageLayout p_new_layout);

        void destroy();

        texture_properties get_properties() const { return m_texture_properties; }

        // VkCommandBuffer get_command_buffer() const { return m_texture_copy_command_buffer; }

        //! @note These are required by textures to copy data into VkBuffer
        //! @note For now, I will implement these and these will get called under the callable lambda in update_uniforms function
        //! @note These will be called but not in this vk_texture class
        // void copy_texture_image(const VkCommandBuffer& p_command_buffer);

        texture_properties load_texture_image_from_data(const void* p_pixels, uint32_t p_width, uint32_t p_height, VkFormat p_format);

        void update_texture_image(texture_properties& p_properties, uint32_t p_width, uint32_t p_height, VkFormat p_format, const void* p_pixels);
        
        /*

        1. begin command buffer
        2. create image barrier
        3. submit copy command buffer (just call submit_copy_command_buffer function afterwards)
        */
        void transition_image_layout(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkFormat p_format, VkImageLayout p_old_layout, VkImageLayout p_new_layout);

        void copy_buffer_to_image(VkCommandBuffer& p_command_buffer, VkImage& p_image, VkBuffer& p_buffer, uint32_t p_width, uint32_t p_height);

        // This should be called outside from main (at first)
        // Either that or just pass copy_command_buffer() into swapchain's submit function directly rather then passing the vk_queue!
        VkCommandBuffer copy_command_buffer() const { return m_texture_copy_command_buffer; }
        void submit_copy_command_buffer();

    private:
        void destroy_staging_buffer();

    private:
        vk_swapchain m_copy_swapchain;
        VkDevice m_driver=nullptr;
        VkCommandPool m_command_pool=nullptr;
        VkCommandBuffer m_texture_copy_command_buffer=nullptr;
        buffer_properties m_texture_staging_buffer_properties{};
        texture_properties m_texture_properties{};
    };
};