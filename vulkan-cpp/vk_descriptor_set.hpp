#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <vulkan-cpp/vk_vertex_buffer.hpp>

namespace vk {
    class vk_descriptor_set {
    public:
        vk_descriptor_set() = default;
        vk_descriptor_set(uint32_t p_num_images);

        void destroy();

        void update_descriptor_sets(const vk_vertex_buffer& p_vertex_buffer);

        VkDescriptorPool get_pool() const { return m_descriptor_pool; }
        VkDescriptorSetLayout get_layout() const { return m_descriptor_set_layout; }

        size_t descriptors_count() const { return m_descriptor_sets.size(); }

        VkDescriptorSet get(uint32_t p_index) const { return m_descriptor_sets[p_index]; }


    private:
        void create_descriptor_pool();

        void create_descriptor_set_layout();

        void allocate_descriptor_sets();

    private:
        uint32_t m_num_images=0;
        VkDevice m_driver=nullptr;

        VkDescriptorPool m_descriptor_pool=nullptr;
        VkDescriptorSetLayout m_descriptor_set_layout=nullptr;
        std::vector<VkDescriptorSet> m_descriptor_sets;
    };
};