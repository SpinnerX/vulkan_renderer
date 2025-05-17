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

    //! @note Reading the raw file and returning the source code read from the shader source file
    static std::string read_file(const std::string& p_filename) {
        std::ifstream ins(p_filename, std::ios::ate | std::ios::binary);

        if (!ins.is_open()) {
            console_log_error("Could not open filename = {}", p_filename);
            return { 'a' };
        }

        size_t file_size = (size_t)ins.tellg();
        // std::vector<char> output(fileSize);
        std::string output;
        output.resize(file_size);

        ins.seekg(0);
        ins.read(output.data(), file_size);

        console_log_trace("output.size() = {}", output.size());

        return output;
    }

    static VkShaderModule create_shader_module(const std::span<uint32_t>& p_binary_blobs) {
        VkDevice driver = vk_driver::driver_context();
        VkShaderModuleCreateInfo module_ci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = p_binary_blobs.size_bytes(),
            .pCode = p_binary_blobs.data()
        };

        VkShaderModule shader_module;
        vk_check(
          vkCreateShaderModule(driver, &module_ci, nullptr, &shader_module),
          "vkCreateShaderModule",
          __FUNCTION__);

        return shader_module;
    }



    /**
     * @param p_shader_src is the actual source of the shader source code in raw text
     * @param p_shader_filename is the filename for ShaderC to compile glsl to spv
     * @note Loads binary blob from the actual shader source, which just returns the binary blob
    */
    static std::vector<uint32_t> load_binary_from_source(const std::string& p_shader_filename, const shader_stage& p_stage) {
        std::string source_raw_text = read_file(p_shader_filename);
        
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan,  shaderc_env_version_vulkan_1_3);
        options.SetWarningsAsErrors();

        shaderc_shader_kind shader_type;
        switch (p_stage){
        case shader_stage::VERTEX:
            shader_type = shaderc_glsl_vertex_shader;
            break;
        case shader_stage::FRAGMENT:
            shader_type = shaderc_glsl_fragment_shader;
            break;
        default:
            break;
        }

        shaderc::Compiler compiler;
        std::vector<uint32_t> binaries;

        //! @note Checks if the sources that are being passed are either hardcoded sources in text or passed in as a file!
        if(std::filesystem::is_regular_file(p_shader_filename)) {
            shaderc::CompilationResult result = compiler.CompileGlslToSpv(
                source_raw_text.data(), 
                source_raw_text.size(), 
                shader_type, 
                p_shader_filename.c_str(),
                "main", 
                options);
            
            //! @note TODO: Should have a better approach at checking if shaderc failed to compile shaders
            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                console_log_error("Shader compilation for {} failed with reason {}.\nReason: {}", p_shader_filename, (int)result.GetCompilationStatus(), result.GetErrorMessage());
                return {};
            }

            for (uint32_t thing : result) {
                binaries.push_back(thing);
            }
        }
        else {
            shaderc::CompilationResult result = compiler.CompileGlslToSpv(
                p_shader_filename.data(), 
                p_shader_filename.size(), 
                shader_type, 
                "builtin",
                "main", 
                options);
            
            //! @note TODO: Should have a better approach at checking if shaderc failed to compile shaders
            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                console_log_error("Shader compilation for {} failed with reason {}.\nReason: {}", p_shader_filename, (int)result.GetCompilationStatus(), result.GetErrorMessage());
                return {};
            }

            for (uint32_t thing : result) {
                binaries.push_back(thing);
            }
        }

        console_log_info("Shader compilation successful for {}!", p_shader_filename);

        return binaries;
    }

    /**
     * @param p_shader_filename is the file of the shader source
     * @param stage is the specific shader stage this shader file is in, the pipeline
     * @note Loads shader file and returns the binary blob from reading the direct source using load_binary_from_source
    */
    static std::vector<uint32_t> load_binary_from_file(const std::string& p_shader_filename, shader_stage stage) {
        return load_binary_from_source(p_shader_filename, stage);
    }


    // vk_shader::vk_shader(VkShaderModule p_vert_module, VkShaderModule p_frag_module) : 
    //     m_vertex_shader_module(p_vert_module),
    //     m_fragment_shader_module(p_frag_module),
    //     m_driver(vk_driver::driver_context()) {
        
    //     // Loading Modules
    // }

    vk_shader::vk_shader(const std::string& p_vert_filename,
                         const std::string& p_frag_filename) {
        
        console_log_info("vk_shader begin loaded shader modules!!!");
        m_driver = vk_driver::driver_context();

        compile(p_vert_filename, p_frag_filename);


        vk_context::submit_resource_free([this](){
            console_log_fatal("vk_shader resource freed");
            vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
            vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);

            m_vertex_shader_module = nullptr;
            m_fragment_shader_module = nullptr;
        });

    }

    void vk_shader::compile(const std::string& p_vert_filename, const std::string& p_frag_filename) {
        
        std::vector<uint32_t> vertex_shader = load_binary_from_file(p_vert_filename, vk::shader_stage::VERTEX);
        std::vector<uint32_t> fragment_shader = load_binary_from_file(p_frag_filename, vk::shader_stage::FRAGMENT);

        if(!std::filesystem::is_regular_file(g_hardcoded_frag)) {
            console_log_fatal("g_hardcoded_frag was not a file!!!");
        }

        if(std::filesystem::is_regular_file(p_vert_filename)) {
            console_log_info("{} is a regular file!!", p_vert_filename);
            console_log_info("{} is a regular file!!", p_frag_filename);
        }
        else {
            console_log_fatal("{} is a NOT regular file!!", p_vert_filename);
            console_log_fatal("{} is a NOT regular file!!", p_frag_filename);
        }

        if (vertex_shader.size() == 0 || fragment_shader.size() == 0) {
            console_log_error("Failed to compile shader!!! Not updating vk_shader!!!!!!!!!!!!");
            
            vertex_shader = load_binary_from_source(g_hardcoded_vert, vk::shader_stage::VERTEX);
            fragment_shader = load_binary_from_source(g_hardcoded_frag, vk::shader_stage::FRAGMENT);
        }

        if (m_vertex_shader_module != nullptr &&
            m_fragment_shader_module != nullptr) {
            destroy();
        }


        // Then we setup the shader module
        m_vertex_shader_module = create_shader_module(vertex_shader);
        m_fragment_shader_module = create_shader_module(fragment_shader);
        
        console_log_info("compiled the shader successfully");
    }

    void vk_shader::destroy() {
        vkDestroyShaderModule(m_driver, m_vertex_shader_module, nullptr);
        vkDestroyShaderModule(m_driver, m_fragment_shader_module, nullptr);

        m_vertex_shader_module = nullptr;
        m_fragment_shader_module = nullptr;
    }

    void vk_shader::set_vertex_bind_attributes(const std::initializer_list<VkVertexInputBindingDescription>& p_attribute_descriptions) {
        m_binding_attribute_descriptions = std::vector<VkVertexInputBindingDescription>(p_attribute_descriptions);
    }

    void vk_shader::set_vertex_attributes(const std::initializer_list<VkVertexInputAttributeDescription>& p_list) {
        m_attribute_descriptions = std::vector<VkVertexInputAttributeDescription>(p_list);
    }
};
