#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_context.hpp>

namespace vk {
    vk_window* vk_window::s_instance = nullptr;
    vk_window::vk_window(const std::string& p_tag, int p_width, int p_height)
      : m_settings(p_width, p_height, p_tag) {
        // m_instance = vk_context::current
        m_instance = vk_context::current_context();
        m_window_handler =
          glfwCreateWindow(p_width, p_height, p_tag.c_str(), nullptr, nullptr);
        s_instance = this;
    }

    vk_window::~vk_window() {
        vkDestroySurfaceKHR(m_instance, m_window_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window_handler);
    }

    void vk_window::center_window() {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        int width = (mode->width / 2) - (m_settings.Width / 2);
        int height = (mode->height / 2) - (m_settings.Height / 2);
        glfwSetWindowPos(m_window_handler, width, height);
    }

    void vk_window::viewport_resize(int p_width, int p_height) {
        m_settings.Width = p_width;
        m_settings.Height = p_height;
    }

    void vk_window::create_window_surface(const VkInstance& p_instance) {
        console_log_info(
          "vk_window::create_window_surface begin initialization!");
        vk::vk_check(
          glfwCreateWindowSurface(
            p_instance, m_window_handler, nullptr, &m_window_surface),
          "glfwCreateWindowSurface",
          __FUNCTION__);
        console_log_info(
          "vk_window::create_window_surface end initialization!!!\n\n");
    }

    void vk_window::clean() {
        vkDestroySurfaceKHR(m_instance, m_window_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window_handler);
    }

    void vk_window::on_create_window() {}
};