#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>

namespace vk {
    vk_descriptor_set::vk_descriptor_set(uint32_t p_num_images, const std::vector<vk_uniform_buffer>& p_uniform_buffers, uint32_t p_data_size_bytes) : m_num_images(p_num_images) {
        m_driver = vk_driver::driver_context();

        create_descriptor_pool();
        create_descriptor_set_layout(p_uniform_buffers, p_data_size_bytes);
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

    void vk_descriptor_set::create_descriptor_set_layout(const std::vector<vk_uniform_buffer>& p_uniform_buffers, uint32_t p_data_size_bytes) {
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

        VkDescriptorSetLayoutBinding vertex_shader_layout_binding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };

        layout_bindings.push_back(vertex_shader_layout_binding);

        VkDescriptorSetLayoutBinding uniform_matrices = {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };
        console_log_trace("size = {}", p_uniform_buffers.size());
        if(p_uniform_buffers.size() > 0) {
            console_log_trace("Do you reach here???");

            layout_bindings.push_back(uniform_matrices);
        }

        console_log_trace("layout_bindings.size() = {}", layout_bindings.size());

        // VkDescriptorSetLayoutBinding fragment_shader_layout_binding = {
        //     .binding = 0,
        //     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        //     .descriptorCount = 1,
        //     .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        // };

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

    void vk_descriptor_set::update_descriptor_sets(const vk_vertex_buffer& p_vertex_buffer, const std::vector<vk_uniform_buffer>& p_uniform_buffer, uint32_t p_uniform_data_size_in_bytes) {
        VkDescriptorBufferInfo descriptor_buffer_info = {
            .buffer = p_vertex_buffer,
            .offset = 0,
            .range = (VkDeviceSize)p_vertex_buffer.size()
        };

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
        }



        vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    void vk_descriptor_set::destroy() {
        vkDestroyDescriptorPool(m_driver, m_descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(m_driver, m_descriptor_set_layout, nullptr);
    }
};