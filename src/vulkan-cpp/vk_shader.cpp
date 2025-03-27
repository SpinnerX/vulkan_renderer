#include <vulkan-cpp/vk_shader.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <array>

namespace vk {

    static std::vector<char> read_file(const std::string& p_filename){
        std::ifstream ins(p_filename, std::ios::ate | std::ios::binary);
        if(!ins){
            return {};
        }
        else{
            console_log_trace("Successfully Read Shader File = {}", p_filename);
        }
    
        size_t size = (size_t)ins.tellg();
    
        std::vector<char> buffer(size);
        ins.seekg(0);
        ins.read(buffer.data(), size);
        ins.close();
    
        return buffer;
    }
    
    VkShaderModule load_shader_module(const VkDevice& p_driver, const std::vector<char>& p_code) {
        VkShaderModuleCreateInfo module_ci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = p_code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(p_code.data())
        };
    
    
        VkShaderModule shader_module;
        vk_check(vkCreateShaderModule(p_driver, &module_ci, nullptr, &shader_module), "vkCreateShaderModule", __FUNCTION__);
    
        return shader_module;
    }

    vk_shader::vk_shader(const std::string& p_vert_filename, const std::string& p_frag_filename) {
        m_driver = vk_driver::driver_context();
        auto vertex_shader = read_file("shaders/vert.spv");
        auto fragment_shader = read_file("shaders/frag.spv");

        // Then we setup the shader module
        m_vertex_shader_module = load_shader_module(m_driver, vertex_shader);
        m_fragment_shader_module = load_shader_module(m_driver, fragment_shader);
    }

    void vk_shader::destroy() {
        vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
        vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);
    }


    void vk_shader::load_from_file(const std::string& p_filename) {
    }

    void vk_shader::load_from_text(const std::string& p_filename) {
    }
};