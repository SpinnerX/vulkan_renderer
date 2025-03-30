#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {

    VkDescriptorType descriptor_set_type(const descriptor_type& p_type) {
        switch (p_type){
        case descriptor_type::STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case descriptor_type::UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case descriptor_type::IMAGE_AND_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        console_log_error("descriptor set type specified is invalid!!!");
    }



    vk_descriptor_set::vk_descriptor_set(uint32_t p_num_images, const std::vector<vk_uniform_buffer>& p_uniform_buffers, uint32_t p_data_size_bytes, vk_texture* p_texture) : m_num_images(p_num_images) {
        m_driver = vk_driver::driver_context();

        create_descriptor_pool();
        create_descriptor_set_layout(p_uniform_buffers, p_data_size_bytes, p_texture);
        allocate_descriptor_sets();
    }

    void vk_descriptor_set::create_descriptor_pool() {
        console_log_trace("begin pool descriptor sets initialization!!");
        VkDescriptorPoolCreateInfo desc_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = m_num_images,
            .poolSizeCount = 0,
            .pPoolSizes = nullptr
        };

        vk_check(vkCreateDescriptorPool(m_driver, &desc_pool_ci, nullptr, &m_descriptor_pool), "vkCreateDescriptorPool", __FUNCTION__);

        console_log_trace("successfully pool descriptor sets initialization!!");
    }

    void vk_descriptor_set::create_descriptor_set_layout(const std::vector<vk_uniform_buffer>& p_uniform_buffers, uint32_t p_data_size_bytes, vk_texture* p_texture) {
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

        VkDescriptorSetLayoutBinding vertex_shader_layout_binding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        };

        layout_bindings.push_back(vertex_shader_layout_binding);

        VkDescriptorSetLayoutBinding uniform_matrices = {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };

        VkDescriptorSetLayoutBinding texture_descriptor_set = {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        };

        console_log_trace("size = {}", p_uniform_buffers.size());
        if(p_uniform_buffers.size() > 0) {
            console_log_trace("Do you reach here???");

            layout_bindings.push_back(uniform_matrices);
        }

        if(p_texture != nullptr) {
            console_log_fatal("Texture is NOT NULL (from descriptors) !!");
            layout_bindings.push_back(texture_descriptor_set);
        }

        console_log_trace("layout_bindings.size() = {}", layout_bindings.size());

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
            .pBindings = layout_bindings.data()
        };

        vk_check(vkCreateDescriptorSetLayout(m_driver, &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout), "vkCreateDescriptorSetLayout", __FUNCTION__);
    }

    void vk_descriptor_set::allocate_descriptor_sets() {
        std::vector<VkDescriptorSetLayout> layouts(m_num_images, m_descriptor_set_layout);

        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptor_pool,
            .descriptorSetCount = m_num_images,
            .pSetLayouts = layouts.data()
        };

        m_descriptor_sets.resize(m_num_images);

        vk_check(vkAllocateDescriptorSets(m_driver, &descriptor_set_alloc_info, m_descriptor_sets.data()), "vkAllocateDescriptorSets", __FUNCTION__);
    }

    void vk_descriptor_set::update_descriptor_sets(const vk_vertex_buffer& p_vertex_buffer, const std::vector<vk_uniform_buffer>& p_uniform_buffer, uint32_t p_uniform_data_size_in_bytes, vk_texture* p_texture) {
        VkDescriptorBufferInfo descriptor_buffer_info = {
            .buffer = p_vertex_buffer,
            .offset = 0,
            .range = (VkDeviceSize)p_vertex_buffer.size()
        };

        VkDescriptorImageInfo texture_descriptor_image_info;

        if(p_texture != nullptr) {
            console_log_fatal("p_texture != nullptr and so we add texture_descriptor_image_info!!!");
            if(p_texture->image_view() != nullptr) {
                console_log_trace("image_view NOT NULL!!!");
            }

            if(p_texture->sampler() != nullptr) {
                console_log_trace("sampler NOT NULL!!!");
            }

            texture_descriptor_image_info = {
                .sampler = p_texture->sampler(),
                .imageView = p_texture->image_view(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }

        std::vector<VkWriteDescriptorSet> write_descriptor_sets;

        for(size_t i = 0; i < m_num_images; i++) {
            
            // (dstBinding should be 0)
            VkWriteDescriptorSet write_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &descriptor_buffer_info
            };

            write_descriptor_sets.push_back(write_descriptor_set);

            if(p_uniform_buffer.size() > 0) {
                VkDescriptorBufferInfo uniform_info = {
                    .buffer = p_uniform_buffer[i],
                    .offset = 0,
                    .range = (VkDeviceSize)p_uniform_data_size_in_bytes
                };

                // (dstBinding should be 1)
                VkWriteDescriptorSet write_descriptor_set = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_descriptor_sets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &uniform_info
                };
    
                write_descriptor_sets.push_back(write_descriptor_set);
            }

            VkWriteDescriptorSet texture_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 2, // Location 2
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &texture_descriptor_image_info,
            };

            if(p_texture != nullptr) {
                console_log_trace("Loading writing descriptor sets for textures!!!");

                write_descriptor_sets.push_back(texture_descriptor_set);
            }
        }

        console_log_trace("DESCRIPTOR SETS WRITING SIZE = {}", write_descriptor_sets.size());

        vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    void vk_descriptor_set::destroy() {
        vkDestroyDescriptorPool(m_driver, m_descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(m_driver, m_descriptor_set_layout, nullptr);
    }
};