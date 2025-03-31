#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/vk_texture.hpp>

namespace vk {
    /*
    descriptor_t: enum
        - STORAGE_BUFFER
        - UNIFORM_BUFFER
        - IMAGE_AND_SAMPLER


    vk_descriptor_set = {
        // since this is the first one it is already assigned do binding = 0
        // or (sampler_count > 0) which is true
        { "inVertices", descriptor_t::STORAGE_BUFFER, .count=1, .has_samplers=false, .stage=shader_stage::VERTEX},

         // since this is the first one it is already assigned do binding = 0
        { "ubo", descriptor_t::UNIFORM_BUFFER, .count=1, .has_samplers=false, .stage=shader_stage::VERTEX},

        { "ubo", descriptor_t::IMAGE_AND_SAMPLER, .count=1, .stage=shader_stage::FRAGMENT},
    }

    
    */


    enum descriptor_type : uint8_t {
        STORAGE_BUFFER=0, // used for writing storage buffers to descriptor sets
        UNIFORM_BUFFER=1, // used for writing uniforms to descriptor sets
        IMAGE_AND_SAMPLER=2 // textures comprise of image_view and sampler that can get written to descriptor sets (via VkDescriptorImageInfo)
    };

    //! TODO: May want this to expand to also have options for geometry, compute, and tesselation shaders
    enum shader_stage : uint8_t {
        VERTEX=0,
        FRAGMENT=1
    };


    /*
    
        Used to construct what our descriptor set layout is going to be
            - Without having us manually describe each descriptor set layout without the verbosity of descriptor sets
            - Used for specifying descriptor set layouts
    */
    struct vk_descriptor_set_properties {
        vk_descriptor_set_properties(const std::string& p_name, const uint32_t p_layout_binding, const descriptor_type& p_type, const shader_stage& p_stage, bool p_has_samplers=false) : Name(p_name), Binding(p_layout_binding), Type(p_type), Stage(p_stage), HasSamplers(p_has_samplers) {}



        std::string Name="Undefined";
        uint32_t Binding=-1;
        descriptor_type Type;
        shader_stage Stage;
        bool HasSamplers=false;
    };



    class vk_descriptor_set {
    public:
        vk_descriptor_set() = default;
        vk_descriptor_set(uint32_t p_num_images, const std::span<vk_uniform_buffer>& p_uniform_buffers, vk_texture* p_texture);
        vk_descriptor_set(uint32_t p_descriptor_count, const std::span<vk_descriptor_set_properties>& p_layouts);

        void destroy();

        void update_descriptor_sets(const vk_vertex_buffer& p_vertex_buffer, const std::span<vk_uniform_buffer>& p_uniform_buffer, vk_texture* p_texture);

        // void write_to_descriptor_set();

        VkDescriptorPool get_pool() const { return m_descriptor_pool; }
        VkDescriptorSetLayout get_layout() const { return m_descriptor_set_layout; }

        size_t descriptors_count() const { return m_descriptor_sets.size(); }

        VkDescriptorSet get(uint32_t p_index) const { return m_descriptor_sets[p_index]; }


    private:
        void create_descriptor_pool();

        void create_descriptor_set_layout(const std::span<vk_uniform_buffer>& p_uniform_buffers, vk_texture* p_texture);

        void allocate_descriptor_sets();

    private:
        uint32_t m_descriptor_count=0;
        VkDevice m_driver=nullptr;

        VkDescriptorPool m_descriptor_pool=nullptr;
        VkDescriptorSetLayout m_descriptor_set_layout=nullptr;
        std::vector<VkDescriptorSet> m_descriptor_sets;
    };
};