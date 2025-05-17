#pragma once
#include <string>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk {
    /**
     * @param p_shader_sources will take in the file to shader source code, also take in the shader stage this shader is utilized at
     * Usage: shader_info shader1 = {"shaders/shader.vert", shader_stage::vertex};
     * 
     * vk_shader_group group1 = {
     *  {"shaders/shader.vert", shader_stage::vertex},
     *  {"shaders/shader.frag", shader_stage::fragment}
     * };
    */

    //! TODO: Once this works make this the primary ways in loading in shdaer sources
    enum shader_stage2 : uint8_t { Vertex = 0, Fragment = 1 };

    struct shader_info {
        std::string filename="Undefined Filename";
        shader_stage2 stage;
    };

    struct shader_module_info {
        VkShaderModule shader_module=nullptr;
        shader_stage2 stage;
    };

    class vk_shader_group {
    public:
        vk_shader_group(const std::initializer_list<shader_info>& p_shader_sources);

        void set_vertex_attributes(const std::initializer_list<VkVertexInputAttributeDescription>& p_list);

        void set_vertex_bind_attributes(const std::initializer_list<VkVertexInputBindingDescription>& p_attribute_descriptions);

        std::span<const shader_module_info> data() const{
            return std::span<const shader_module_info>(m_shader_modules);
        }

        std::span<VkVertexInputAttributeDescription> get_vertex_attributes() { return m_attribute_descriptions; }
        std::span<VkVertexInputBindingDescription> get_vertex_bind_attributes() { return m_binding_attribute_descriptions; }


        //! @note Option to recompile shaders with newly specified shader sources
        void compile(const std::initializer_list<shader_info>& p_shader_sources);

        //! @note Compile shaders if there are shader sources provided
        void compile();
    private:
        VkDevice m_driver = nullptr;
        std::vector<shader_module_info> m_shader_modules{};
        std::vector<shader_info> m_shader_sources{};
        std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions{};
        std::vector<VkVertexInputBindingDescription> m_binding_attribute_descriptions{};
    };
};