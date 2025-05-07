#include <shaderc/status.h>
#include <vulkan-cpp/vk_shader.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <fstream>
#include <fmt/ranges.h>
#include <shaderc/shaderc.hpp>
#include <shader_compiler/shader_compiler.hpp>
#include <vulkan-cpp/vk_context.hpp>


namespace vk {

    static const std::string g_hardcoded_vert = 
        "#version 460\n"
        "layout(location = 0) in vec3 inPosition;\n"
        "layout(location = 1) in vec3 inColor;\n"
        "layout(location = 2) in vec3 inNormals;\n"
        "layout(location = 3) in vec2 inTexCoords;\n"
        "layout(location = 0) out vec4 fragColor;\n"
        "layout(location = 1) out vec3 fragNormals;\n"
        "layout(location = 2) out vec2 fragTexCoords;\n"
        "layout (binding = 0) uniform UniformBuffer {\n"
        "	mat4 MVP;\n"
        "} ubo;\n"

        "void main() {\n"
        "	gl_Position = ubo.MVP * vec4(inPosition, 1.0);\n"
        "	fragColor = vec4(inColor, 1.0);\n"
        "	fragTexCoords = inTexCoords;\n"
        "	fragNormals = inNormals;\n"
        "}\n";
    static const std::string g_hardcoded_frag = 
        "#version 460\n"
        "layout (binding = 1) uniform sampler2D texSampler;\n"
        "layout (location = 0) in vec4 fragColor;\n"
        "layout (location = 1) in vec3 fragNormals;\n"
        "layout (location = 2) in vec2 fragTexCoords;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "    vec3 col1 = vec3(1.0, 0.0, 1.0);\n"
        "    vec3 col2 = vec3(0.0, 0.0, 0.0);\n"
        "    ivec2 tile = ivec2(fragTexCoords * 10.0f);\n"
        "    tile.x += tile.y % 2;\n"
        "    ivec2 stuff = tile % 2;\n"
        "    outColor = vec4(col1 * float(stuff.x) + col2 * float(1 - stuff.y), 1.0);\n"
        "};\n";

    static VkShaderModule load_shader_module(const VkDevice& p_driver, const std::span<uint32_t>& p_binary_blobs);

    static std::vector<char> read_file(const std::string& p_filename);

    static std::vector<uint32_t> load_binary_from_source(const std::string& p_shader_src, const std::string& p_shader_filename, shader_stage stage) {
        
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
            p_shader_src.data(), 
            p_shader_src.size(), 
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

    
    static std::vector<uint32_t> load_binary_from_file(const std::string& p_shader_filename, shader_stage stage) {
        std::vector<char> source_text = read_file(p_shader_filename);
        std::string source_string(source_text.begin(), source_text.end());
        return load_binary_from_source(source_string, p_shader_filename, stage);
    }
    

    vk_shader vk_shader::from_source_files(const std::string &p_vert_filename, const std::string &p_frag_filename) {
        std::vector<uint32_t> vert_binaries = load_binary_from_file(p_vert_filename, shader_stage::VERTEX);
        std::vector<uint32_t> frag_binaries = load_binary_from_file(p_frag_filename, shader_stage::FRAGMENT);

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


        vk_context::submit_resource_free([this](){
            console_log_fatal("vk_shader resource freed");
            vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
            vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);

            m_vertex_shader_module = VK_NULL_HANDLE;
            m_fragment_shader_module = VK_NULL_HANDLE;
        });

    }

    void vk_shader::compile(const std::string& p_vert_filename, const std::string& p_frag_filename) {
        
        std::vector<uint32_t> vertex_shader = load_binary_from_file(p_vert_filename, vk::shader_stage::VERTEX);
        std::vector<uint32_t> fragment_shader = load_binary_from_file(p_frag_filename, vk::shader_stage::FRAGMENT);

        if (vertex_shader.size() == 0 || fragment_shader.size() == 0) {
            console_log_error("Failed to compile shader!!! Not updating vk_shader!!!!!!!!!!!!");
            
            vertex_shader = load_binary_from_source(g_hardcoded_vert, "builtin", vk::shader_stage::VERTEX);
            fragment_shader = load_binary_from_source(g_hardcoded_frag, "builtin", vk::shader_stage::FRAGMENT);
        }

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
