#include <vulkan-cpp/uniforms.hpp>
#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <span>
#include <vulkan-cpp/vk_context.hpp>

namespace vk {

    VkDescriptorType to_descriptor_set_type(const descriptor_type& p_type) {
        switch (p_type) {
            case descriptor_type::STORAGE_BUFFER:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case descriptor_type::UNIFORM_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case descriptor_type::IMAGE_AND_SAMPLER:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        console_log_error("descriptor set type specified is invalid!!!");
    }

    VkShaderStageFlags to_vk_shader_stage(const shader_stage& p_stage) {
        switch (p_stage) {
            case shader_stage::VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case shader_stage::FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        console_log_error(
          "vulkan shader stage that you specified was invalid!!!");
    }

    vk_descriptor_set::vk_descriptor_set(
      uint32_t p_descriptor_count,
      const std::initializer_list<VkDescriptorSetLayoutBinding>& p_layouts)
      : m_descriptor_count(p_descriptor_count) {
        m_driver = vk_driver::driver_context();

        console_log_trace("begin pool descriptor sets initialization!!");
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_descriptor_count);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_descriptor_count);
        VkDescriptorPoolCreateInfo desc_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = m_descriptor_count,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        vk_check(vkCreateDescriptorPool(
                   m_driver, &desc_pool_ci, nullptr, &m_descriptor_pool),
                 "vkCreateDescriptorPool",
                 __FUNCTION__);

        console_log_trace("successfully pool descriptor sets initialization!!");

        // automate -- setting up descriptor set layouts
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings(p_layouts);
        
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
            .pBindings = layout_bindings.data()
        };

        vk_check(vkCreateDescriptorSetLayout(m_driver,
                                             &descriptor_set_layout_ci,
                                             nullptr,
                                             &m_descriptor_set_layout),
                 "vkCreateDescriptorSetLayout",
                 __FUNCTION__);

        // Now that we setup the layouts we can just setup now start allocating
        // based on our layout setup
        std::vector<VkDescriptorSetLayout> layouts(m_descriptor_count,
                                                   m_descriptor_set_layout);

        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptor_pool,
            .descriptorSetCount = m_descriptor_count,
            .pSetLayouts = layouts.data()
        };

        m_descriptor_sets.resize(m_descriptor_count);

        vk_check(vkAllocateDescriptorSets(m_driver,
                                          &descriptor_set_alloc_info,
                                          m_descriptor_sets.data()),
                 "vkAllocateDescriptorSets",
                 __FUNCTION__);
        vk_context::submit_resource_free([this](){
            vkDestroyDescriptorPool(m_driver, m_descriptor_pool, nullptr);
            vkDestroyDescriptorSetLayout(m_driver, m_descriptor_set_layout, nullptr);
        });
    }


    void vk_descriptor_set::bind(const VkCommandBuffer& p_command_buffer,
                                 uint32_t p_frame_index,
                                 const VkPipelineLayout& p_pipeline_layout) {

        if (m_descriptor_sets.size() > 0) {
            vkCmdBindDescriptorSets(p_command_buffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    p_pipeline_layout,
                                    0,
                                    1,
                                    &m_descriptor_sets[p_frame_index],
                                    0,
                                    nullptr);
        }
    }

    // void vk_descriptor_set::write(const std::span<vk_uniform_buffer>& p_uniforms) {
        
    // }

    // void vk_descriptor_set::write(const vk_uniform_buffer& p_uniform) {}

    // void vk_descriptor_set::write(const vk_vertex_buffer& p_vbo, const std::span<vk_texture>& p_textures) {}

    void vk_descriptor_set::update_mesh(const std::span<vk_uniform_buffer>& p_uniforms, const mesh& p_mesh) {
        for(size_t i = 0; i < m_descriptor_count; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = p_uniforms[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(camera_data_uniform);

            // std::array<vk_texture, 4> textures;
            std::vector<VkDescriptorImageInfo> descriptor_image_infos(p_mesh.texture_size());

            for(size_t i = 0; i < p_mesh.texture_size(); i++) {
                vk_texture texture = p_mesh.get_texture(i);
                VkDescriptorImageInfo image_info{};
                image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info.imageView = texture.image_view();
                image_info.sampler = texture.sampler();
                // descriptor_image_infos.emplace_back(image_info);
                descriptor_image_infos[i] = image_info;
            }

            // std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            std::array<VkWriteDescriptorSet, 2> write_descriptors;

            write_descriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[0].pNext = nullptr;
            write_descriptors[0].dstSet = m_descriptor_sets[i];
            write_descriptors[0].dstBinding = 0;
            write_descriptors[0].dstArrayElement = 0;
            write_descriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptors[0].descriptorCount = 1;
            write_descriptors[0].pBufferInfo = &bufferInfo;

            //! TODO: Because this takes in only one texture meaning that we can only pass in one texture
            //! TODO: Unless we pass in explicitly 4 texture slots to the mesh that can be utilized by this specific write descriptor
            write_descriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[1].pNext = nullptr;
            write_descriptors[1].dstSet = m_descriptor_sets[i];
            write_descriptors[1].dstBinding = 1;
            write_descriptors[1].dstArrayElement = 0;
            write_descriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptors[1].descriptorCount = 1;
            write_descriptors[1].pImageInfo = &descriptor_image_infos[0];
            vkUpdateDescriptorSets(m_driver, static_cast<uint32_t>(write_descriptors.size()), write_descriptors.data(), 0, nullptr); 
        }
    }

    void vk_descriptor_set::destroy() {
        // vkDestroyDescriptorPool(m_driver, m_descriptor_pool, nullptr);
        // vkDestroyDescriptorSetLayout(m_driver, m_descriptor_set_layout, nullptr);
    }
};