#include <shaderc/shaderc.hpp>
#include <shader_compiler/shader_compiler.hpp>
#include <vulkan-cpp/vk_shader_group.hpp>
#include <fstream>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <filesystem>

namespace vk {

    std::string read_file(const std::string& p_filename) {
        std::ifstream ins(p_filename, std::ios::ate | std::ios::binary);

        if(!ins) {
            console_log_error("Failed to load file {}", p_filename);
            return {};
        }

        size_t file_size = (size_t)ins.tellg();
        std::string output="";
        output.resize(file_size);
        ins.seekg(0);
        ins.read(output.data(), file_size);
        return output;
    }

    static VkShaderModule create_shader_module(const std::span<uint32_t>& p_binary_blob) {
        VkDevice driver = vk_driver::driver_context();
        VkShaderModuleCreateInfo module_ci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = p_binary_blob.size_bytes(),
            .pCode = p_binary_blob.data()
        };

        VkShaderModule shader_module;
        vk_check(
          vkCreateShaderModule(driver, &module_ci, nullptr, &shader_module),
          "vkCreateShaderModule",
          __FUNCTION__);

        return shader_module;
    }

    //! @note Loads in the shader source
    //! @note Also does a check if the shader source we are loading
    static std::vector<uint32_t> load_shader_source(const std::string& p_filename, const shader_stage2& p_stage) {


        std::vector<uint32_t> binary_blob{};

        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetWarningsAsErrors();


        shaderc_shader_kind shader_t;
        switch (p_stage){
        case shader_stage2::Vertex:
            shader_t = shaderc_glsl_vertex_shader;
            break;
        case shader_stage2::Fragment:
            shader_t = shaderc_glsl_fragment_shader;
            break;
        default:
            break;
        }

        shaderc::Compiler compiler;
        
        //! @note Read from that file if the parameter is a file itself
        if(std::filesystem::is_regular_file(p_filename)) {
            std::string shader_raw_source_text = read_file(p_filename);

            shaderc::CompilationResult result = compiler.CompileGlslToSpv(
                shader_raw_source_text.data(),
                shader_raw_source_text.size(),
                shader_t,
                p_filename.c_str(),
                "main",
                options
            );

            if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
                // console_log_error("Shader compilation failed for {}", p_filename);
                console_log_error("Shader compilation for {} failed with reason {}.", p_filename, (int)result.GetCompilationStatus(), result.GetErrorMessage());
                return {};
            }

            for(const uint32_t& block : result) {
                binary_blob.push_back(block);
            }
        }
        else { // executes if the user passes in hardcoded shader code
            shaderc::CompilationResult result = compiler.CompileGlslToSpv(
                p_filename.data(), 
                p_filename.size(), 
                shader_t, 
                "builtin",
                "main", 
                options);
            
            //! @note TODO: Should have a better approach at checking if shaderc failed to compile shaders
            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                console_log_error("Shader compilation for {} failed with reason {}.", p_filename, (int)result.GetCompilationStatus(), result.GetErrorMessage());
                return {};
            }

            for (uint32_t thing : result) {
                binary_blob.push_back(thing);
            }
        }

        return binary_blob;
    }

    vk_shader_group::vk_shader_group(const std::initializer_list<shader_info>& p_shader_sources) : m_shader_sources(p_shader_sources) {
        m_driver = vk_driver::driver_context();
        compile();

        vk_context::submit_resource_free([this](){
            console_log_error("vk_shader_group freed!");
            for(size_t i = 0; i < m_shader_modules.size(); i++) {
                vkDestroyShaderModule(m_driver, m_shader_modules[i].shader_module, nullptr);
            }
        });
    }

    void vk_shader_group::set_vertex_attributes(const std::initializer_list<VkVertexInputAttributeDescription>& p_list) {
        m_attribute_descriptions = std::vector<VkVertexInputAttributeDescription>(p_list);
    }

    void vk_shader_group::set_vertex_bind_attributes(const std::initializer_list<VkVertexInputBindingDescription>& p_attribute_descriptions) {
        m_binding_attribute_descriptions = std::vector<VkVertexInputBindingDescription>(p_attribute_descriptions);
    }

    //! @note used for compiling the shaders with the specified shader sources if already provided
    void vk_shader_group::compile() {
        for(shader_info info : m_shader_sources) {
            std::vector<uint32_t> binary_blobs = load_shader_source(info.filename, info.stage);
            if(!binary_blobs.empty()) {
                shader_module_info module_info = {
                    .shader_module = create_shader_module(binary_blobs),
                    .stage = info.stage
                };
                m_shader_modules.push_back(module_info);
            }
        }

        console_log_fatal("M_SHADER_SOURCES.SIZE() = {}", m_shader_sources.size());
        console_log_fatal("M_SHADER_MODULES.SIZE() = {}", m_shader_modules.size());
    }

    //! @note Used for compiling shader sources if there are no sources already provided or override already-provided shader sources with new shader sources
    void vk_shader_group::compile(const std::initializer_list<shader_info>& p_shader_sources) {
        m_shader_sources = std::vector<shader_info>(p_shader_sources);
        compile();
    }

};