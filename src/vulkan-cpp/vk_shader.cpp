#include <shaderc/status.h>
#include <vulkan-cpp/vk_shader.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <fstream>
#include <fmt/ranges.h>
#include <shaderc/shaderc.hpp>
#include <shader_compiler/shader_compiler.hpp>

namespace vk {

    static VkShaderModule load_shader_module(const VkDevice& p_driver, const std::span<uint32_t>& p_binary_blobs);

    static std::vector<char> read_file(const std::string& p_filename);

    static std::vector<uint32_t> load_binary_from_source(const std::string &p_shader_filename, shader_stage stage) {
        std::vector<char> source_text = read_file(p_shader_filename);
        
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan,  shaderc_env_version_vulkan_1_3);
        options.SetWarningsAsErrors();

        shaderc_shader_kind shader_type;
        if (stage == shader_stage::VERTEX) {
            shader_type = shaderc_glsl_vertex_shader;
        } else {
            shader_type = shaderc_glsl_fragment_shader;
        }
        

        shaderc::Compiler compiler;
        shaderc::CompilationResult result = compiler.CompileGlslToSpv(
            source_text.data(), 
            source_text.size(), 
            shader_type, 
            p_shader_filename.c_str(), 
            "main", 
            options);
        
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            console_log_error("Shader compilation for {} failed with reason {}.\nReason: {}", p_shader_filename, (int)result.GetCompilationStatus(), result.GetErrorMessage());
            return {};
        }

        console_log_info("Shader compilation successful for {}!", p_shader_filename);

        std::vector<uint32_t> binaries;

       for (auto thing : result) {
            binaries.push_back(thing);
       }
        

        console_log_info("If your seeing this then i did not segfault");
        return binaries;
    }

    vk_shader vk_shader::from_source_files(const std::string &p_vert_filename, const std::string &p_frag_filename) {
        std::vector<uint32_t> vert_binaries = load_binary_from_source(p_vert_filename, shader_stage::VERTEX);
        std::vector<uint32_t> frag_binaries = load_binary_from_source(p_frag_filename, shader_stage::FRAGMENT);

        if (vert_binaries.size() == 0 || frag_binaries.size() == 0) {
            console_log_error("binary lengths invalid (vert: {} frag: {})",
                              vert_binaries.size(),
                              frag_binaries.size());
            // uh I can't return error oops
            // longjmp(off_a_bridge);
        }

        vk_driver driver = vk_driver::driver_context();
        VkShaderModule vert_module = load_shader_module(driver, vert_binaries);
        VkShaderModule frag_module = load_shader_module(driver, frag_binaries);

        vk_shader new_shader(vert_module, frag_module);

        return new_shader;
    }


    static std::vector<char> read_file(const std::string& p_filename) {
        std::ifstream ins(p_filename, std::ios::ate | std::ios::binary);

        if (!ins.is_open()) {
            console_log_error("Could not open filename = {}", p_filename);
            return { 'a' };
        }

        size_t fileSize = (size_t)ins.tellg();
        std::vector<char> output(fileSize);
        ins.seekg(0);
        ins.read(output.data(), fileSize);

        console_log_trace("output.size() = {}", output.size());

        return output;
    }

    static VkShaderModule load_shader_module(const VkDevice& p_driver,
                                             const std::span<uint32_t>& p_binary_blobs) {
        VkShaderModuleCreateInfo module_ci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = p_binary_blobs.size() * sizeof(uint32_t),
            .pCode = p_binary_blobs.data()
        };

        VkShaderModule shader_module;
        vk_check(
          vkCreateShaderModule(p_driver, &module_ci, nullptr, &shader_module),
          "vkCreateShaderModule",
          __FUNCTION__);

        return shader_module;
    }
    
    static VkShaderModule load_shader_module(const VkDevice& p_driver,
                                             const std::span<char>& p_code) {
        return VK_NULL_HANDLE;
    }

    vk_shader::vk_shader(VkShaderModule p_vert_module, VkShaderModule p_frag_module) : 
        m_vertex_shader_module(p_vert_module),
        m_fragment_shader_module(p_frag_module),
        m_driver(vk_driver::driver_context())
    {}

    vk_shader::vk_shader(const std::string& p_vert_filename,
                         const std::string& p_frag_filename) {
        console_log_info("vk_shader begin loaded shader modules!!!");
        m_driver = vk_driver::driver_context();

        if (m_driver != nullptr) {
            console_log_trace("m_driver is in fact valid!!!!");
        }

        compile(p_vert_filename, p_frag_filename);

    }

    void vk_shader::compile(const std::string& p_vert_filename, const std::string& p_frag_filename) {
        
        std::vector<uint32_t> vertex_shader = load_binary_from_source(p_vert_filename, vk::shader_stage::VERTEX);
        std::vector<uint32_t> fragment_shader = load_binary_from_source(p_frag_filename, vk::shader_stage::FRAGMENT);

        if (m_vertex_shader_module != VK_NULL_HANDLE &&
            m_fragment_shader_module != VK_NULL_HANDLE) {
            destroy();
        }


        // Then we setup the shader module
        m_vertex_shader_module = load_shader_module(m_driver, vertex_shader);
        m_fragment_shader_module =
          load_shader_module(m_driver, fragment_shader);
        
        console_log_info("compiled the shader successfully");
    }

    void vk_shader::destroy() {
        vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
        vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);

        m_vertex_shader_module = VK_NULL_HANDLE;
        m_fragment_shader_module = VK_NULL_HANDLE;
    }

    void vk_shader::load_from_file(const std::string& p_filename) {}

    void vk_shader::load_from_text(const std::string& p_filename) {}


    void vk_shader::set_vertex_bind_attributes(const std::initializer_list<VkVertexInputBindingDescription>& p_attribute_descriptions) {
        m_binding_attribute_descriptions = std::vector<VkVertexInputBindingDescription>(p_attribute_descriptions);
    }

    void vk_shader::set_vertex_attributes(const std::initializer_list<VkVertexInputAttributeDescription>& p_list) {
        m_attribute_descriptions = std::vector<VkVertexInputAttributeDescription>(p_list);
    }
};
