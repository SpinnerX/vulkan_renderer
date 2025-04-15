#include <shader_compiler/shader_compiler.hpp>

/* test cpp file */
#include "vulkan-cpp/logger.hpp"
#include <algorithm>

namespace vk {
shader_compilation_context::shader_compilation_context(shader_stage p_stage) : 
    m_shader_stage(p_stage)
{}

void shader_compilation_context::load_source_text(std::span<char> p_source) {
    m_source_text = std::vector<char>(p_source.size());
    
    std::copy(p_source.begin(), p_source.end(), m_source_text.begin());

}

shader_compile_result
shader_compilation_context::compile() {
    
    return {};
}

std::vector<char> shader_compile_result::take_binary_bytes() {
    if (m_binary_buffer.has_value()) {
        return std::move(m_binary_buffer.value());
    }

    return {};
}
}
