#pragma once
/* Just test*/

#include <optional>
#include <string>
#include <span>
#include <memory>
#include <memory>

#include "vulkan-cpp/vk_descriptor_set.hpp"

namespace vk {

class shader_compilation_context;
class shader_compile_result;

enum class shader_compile_status {
    OK,
    FAILURE
};
    
// storage container class to hold
// shader metadata and facilitate compilation of 
// shader source across all stages.
class shader_compilation_context {
public:
    shader_compilation_context(shader_stage p_stage);

    shader_compile_result compile();
    
    void load_source_text(std::span<char> p_source);
private:
    std::vector<char> m_source_text;
    shader_stage m_shader_stage;
};

class shader_compile_result {
public:
    shader_compile_status status() {
        return m_status;
    }

    const std::string& status_msg() {
        return m_status_msg;
    }

    std::vector<char> take_binary_bytes();

private:
    shader_compile_status m_status = shader_compile_status::OK;
    std::string m_status_msg = "";

    std::optional<std::vector<char>> m_binary_buffer;
};
}
