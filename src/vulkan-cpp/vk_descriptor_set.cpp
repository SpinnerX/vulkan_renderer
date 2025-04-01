#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {

    VkDescriptorType to_descriptor_set_type(const descriptor_type& p_type) {
        switch (p_type){
        case descriptor_type::STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case descriptor_type::UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case descriptor_type::IMAGE_AND_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        console_log_error("descriptor set type specified is invalid!!!");
    }

    VkShaderStageFlags to_vk_shader_stage(const shader_stage& p_stage) {
        switch (p_stage){
        case shader_stage::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case shader_stage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        console_log_error("vulkan shader stage that you specified was invalid!!!");
    }


    vk_descriptor_set::vk_descriptor_set(uint32_t p_descriptor_count, const std::span<vk_descriptor_set_properties>& p_layouts) : m_descriptor_count(p_descriptor_count) {
        m_driver = vk_driver::driver_context();

        console_log_trace("begin pool descriptor sets initialization!!");
        VkDescriptorPoolCreateInfo desc_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = m_descriptor_count,
            .poolSizeCount = 0,
            .pPoolSizes = nullptr
        };

        vk_check(vkCreateDescriptorPool(m_driver, &desc_pool_ci, nullptr, &m_descriptor_pool), "vkCreateDescriptorPool", __FUNCTION__);

        console_log_trace("successfully pool descriptor sets initialization!!");


        // automate -- setting up descriptor set layouts
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
        for(const auto& layout : p_layouts) {

            VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {
                .binding = (uint32_t)layout.Binding,
                .descriptorType = to_descriptor_set_type(layout.Type),
                .descriptorCount = 1,
                .stageFlags = to_vk_shader_stage(layout.Stage)
            };

            layout_bindings.push_back(descriptor_set_layout_binding);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
            .pBindings = layout_bindings.data()
        };

        vk_check(vkCreateDescriptorSetLayout(m_driver, &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout), "vkCreateDescriptorSetLayout", __FUNCTION__);


        // Now that we setup the layouts we can just setup now start allocating based on our layout setup
        std::vector<VkDescriptorSetLayout> layouts(m_descriptor_count, m_descriptor_set_layout);

        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptor_pool,
            .descriptorSetCount = m_descriptor_count,
            .pSetLayouts = layouts.data()
        };

        m_descriptor_sets.resize(m_descriptor_count);

        vk_check(vkAllocateDescriptorSets(m_driver, &descriptor_set_alloc_info, m_descriptor_sets.data()), "vkAllocateDescriptorSets", __FUNCTION__);
    }

    void vk_descriptor_set::bind(const VkCommandBuffer& p_command_buffer, uint32_t p_frame_index, const VkPipelineLayout& p_pipeline_layout) {

        if(m_descriptor_sets.size() > 0) {
            vkCmdBindDescriptorSets(p_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_pipeline_layout, 0, 1, &m_descriptor_sets[p_frame_index], 0, nullptr);
        }
    }

    void vk_descriptor_set::update_uniforms(const std::span<vk_uniform_buffer>& p_uniform_buffer) {
        
        std::vector<VkWriteDescriptorSet> write_descriptor_sets;
        for(size_t i = 0; i < m_descriptor_count; i++) {
            if(p_uniform_buffer.size() > 0) {
                VkDescriptorBufferInfo uniform_info = {
                    .buffer = p_uniform_buffer[i],
                    .offset = 0,
                    .range = (VkDeviceSize)p_uniform_buffer.size_bytes()
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
        }

        // Apply our texture to all descriptor sets
        // for(size_t i = 0; i < m_descriptor_count; i++) {
        //     write_descriptor_set.dstSet = m_descriptor_sets[i];
        // }

        vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    void vk_descriptor_set::update_vertex(const vk_vertex_buffer& p_vertex_buffer) {
        std::vector<VkWriteDescriptorSet> write_descriptor_sets;
        VkDescriptorBufferInfo descriptor_buffer_info = {
            .buffer = p_vertex_buffer,
            .offset = 0,
            .range = (VkDeviceSize)p_vertex_buffer.size()
        };

        for(size_t i = 0; i < m_descriptor_count; i++) {
            VkWriteDescriptorSet vertex_buffer_write_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &descriptor_buffer_info
            };
            write_descriptor_sets.push_back(vertex_buffer_write_descriptor_set);
        }

        vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    void vk_descriptor_set::update_texture(const vk_texture* p_texture) {
        std::vector<VkWriteDescriptorSet> write_descriptor_sets;
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

        // Apply our texture to all descriptor sets
        for(size_t i = 0; i < m_descriptor_count; i++) {
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

        vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    void vk_descriptor_set::update_write_descriptors() {
        // vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(m_write_descriptor_sets.size()), m_write_descriptor_sets.data(), 0, nullptr);
    }


    void vk_descriptor_set::destroy() {
        vkDestroyDescriptorPool(m_driver, m_descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(m_driver, m_descriptor_set_layout, nullptr);
    }
};