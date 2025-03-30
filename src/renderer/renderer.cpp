#include <renderer/renderer.hpp>

renderer* renderer::s_instance = nullptr;
std::deque<std::function<void(const VkCommandBuffer&)>> renderer::s_tasks;
std::deque<std::function<void(VkCommandBuffer&)>> renderer::s_deletion_tasks;

renderer::renderer(const std::string& p_tag) {
    s_instance = this;
}

void renderer::begin() {
}

void renderer::end() {
}