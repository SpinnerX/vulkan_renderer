#include <renderer/renderer.hpp>

renderer* renderer::s_instance = nullptr;

renderer::renderer(const std::string& p_tag) {
    s_instance = this;
}