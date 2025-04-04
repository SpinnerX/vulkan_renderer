#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <string>
#include <span>
#include <initializer_list>

namespace vk {
    enum class shader_load_type { File = 0, Text = 1 };

    /*
    struct shader_vetex_attribute {
        std::string Name="Undefined";
        VkVertexInputAttributeDescription AttributeDescription;
    };

    struct shader_bind_vetex_attribute {
        std::string Name="Undefined";
        VkVertexInputBindingDescription AttributeDescription;
    };
    */

    class vk_shader {
    public:
        vk_shader(const std::string& p_vert_filename,
                  const std::string& p_frag_filename);

        // VkPipeline get_graphics_pipeline() { return m_graphics_pipeline; }

        VkShaderModule get_vertex_module() const {
            return m_vertex_shader_module;
        }
        VkShaderModule get_fragment_module() const {
            return m_fragment_shader_module;
        }

        void destroy();

        void set_window_size(uint32_t p_width, uint32_t p_height) {
            m_window_size = { p_width, p_height };
        }

    
        void set_vertex_attributes(const std::initializer_list<VkVertexInputAttributeDescription>& p_list);
        void set_vertex_bind_attributes(const std::initializer_list<VkVertexInputBindingDescription>& p_attribute_descriptions);

        std::span<VkVertexInputAttributeDescription> get_vertex_attributes() { return m_attribute_descriptions; }
        std::span<VkVertexInputBindingDescription> get_vertex_bind_attributes() { return m_binding_attribute_descriptions; }

    private:
        void load_from_file(const std::string& p_filename);
        void load_from_text(const std::string& p_filename);

    private:
        vk_driver m_driver;
        VkShaderModule m_vertex_shader_module = nullptr;
        VkShaderModule m_fragment_shader_module = nullptr;
        VkExtent2D m_window_size{};

        std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;
        std::vector<VkVertexInputBindingDescription> m_binding_attribute_descriptions;
    };
};