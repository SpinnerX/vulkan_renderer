#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/vk_texture.hpp>
#include <renderer/mesh.hpp>
#include <vulkan-cpp/vk_vertex_buffer.hpp>

namespace vk {
    /*
    descriptor_t: enum
        - STORAGE_BUFFER
        - UNIFORM_BUFFER
        - IMAGE_AND_SAMPLER


    vk_descriptor_set = {
        // since this is the first one it is already assigned do binding = 0
        // or (sampler_count > 0) which is true
        { "inVertices", descriptor_t::STORAGE_BUFFER, .count=1,
    .has_samplers=false, .stage=shader_stage::VERTEX},

         // since this is the first one it is already assigned do binding = 0
        { "ubo", descriptor_t::UNIFORM_BUFFER, .count=1, .has_samplers=false,
    .stage=shader_stage::VERTEX},

        { "ubo", descriptor_t::IMAGE_AND_SAMPLER, .count=1,
    .stage=shader_stage::FRAGMENT},
    }


    */

    enum descriptor_type : uint8_t {
        STORAGE_BUFFER =
          0, // used for writing storage buffers to descriptor sets
        UNIFORM_BUFFER = 1, // used for writing uniforms to descriptor sets
        IMAGE_AND_SAMPLER =
          2 // textures comprise of image_view and sampler that can get written
            // to descriptor sets (via VkDescriptorImageInfo)
    };

    //! TODO: May want this to expand to also have options for geometry,
    //! compute, and tesselation shaders
    enum shader_stage : uint8_t { VERTEX = 0, FRAGMENT = 1 };

    /*

        For now lets just supply write descriptor sets as a
       std::span<vk_descriptor_set> and const vk_descriptor_set&
       p_descriptor_set

        std::span<vk_descriptor_set> can be used for things such as using entire
       uniforms that gets appled throughout all processed images

        vk_descriptor_set can be updated for individual descriptors associating
       to specific uniforms


    */

    enum write_descriptor_type : uint8_t {
        WRITE_STORAGE_BUFFER = 0,
        WRITE_UNIFORM_BUFFER = 1,
        TEXTURE_IMAGE_AND_SAMPLER = 2
    };

    /*

        Used to construct what our descriptor set layout is going to be
            - Without having us manually describe each descriptor set layout
       without the verbosity of descriptor sets
            - Used for specifying descriptor set layouts
    */
    struct vk_descriptor_set_properties {
        vk_descriptor_set_properties(const std::string& p_name,
                                     const uint32_t p_layout_binding,
                                     const descriptor_type& p_type,
                                     const shader_stage& p_stage,
                                     bool p_has_samplers = false)
          : Name(p_name)
          , Binding(p_layout_binding)
          , Type(p_type)
          , Stage(p_stage)
          , HasSamplers(p_has_samplers) {}

        std::string Name = "Undefined";
        uint32_t Binding = -1;
        descriptor_type Type;
        shader_stage Stage;
        bool HasSamplers = false;
    };

    struct write_descriptor_sets {
        std::string Name = "Undefined";
    };

    /*

        descriptor_set[i].update_descriptor_set(uniform_buffer[i]);

        Lets also rewrite this to support multiple descriptor_set layouts
        Abstraction for vk_descriptor_set_layout



        API Idea

        If I wanted to do this m_shader.set("position", glm::vec3());
            - I would need a set of descriptor sets
            - Associating a name with the specific descriptor sets we want to
       update


    */

    class vk_descriptor_set {
    public:
        vk_descriptor_set() = default;
        vk_descriptor_set(
          uint32_t p_descriptor_count,
          const std::initializer_list<VkDescriptorSetLayoutBinding>& p_layouts);

        //! @note Does cleanup for descriptor set
        void destroy();

        void bind(const VkCommandBuffer& p_command_buffer,
                  uint32_t p_frame_index,
                  const VkPipelineLayout& p_pipeline_layout);

        // Updating specific groups of descriptor sets
        //! @note Reason these are getting called for every descriptor set
        //! @note Its because they need to be applied when doing camera
        //! transforms, etc.
        void update_uniforms(
          const std::span<vk_uniform_buffer>& p_uniform_buffer);
        void update_vertex(const vk_vertex_buffer& p_vertex_buffer);
        void update_texture(const vk_texture* p_texture);

        // void update_test_descriptors(const std::initializer_list<VkWriteDescriptorSet>& p_write_descriptors);
        void update_test_descriptors(const std::span<vk_uniform_buffer>& p_uniforms, vk_vertex_buffer& p_vertex, vk_texture& p_texture);

        VkDescriptorPool get_pool() const { return m_descriptor_pool; }
        VkDescriptorSetLayout get_layout() const {
            return m_descriptor_set_layout;
        }

        size_t descriptors_count() const { return m_descriptor_sets.size(); }

        VkDescriptorSet get(uint32_t p_index) const {
            return m_descriptor_sets[p_index];
        }

    private:
        uint32_t m_descriptor_count = 0;
        VkDevice m_driver = nullptr;
        VkDescriptorPool m_descriptor_pool = nullptr;
        VkDescriptorSetLayout m_descriptor_set_layout = nullptr;
        std::vector<VkDescriptorSet> m_descriptor_sets;
    };
};