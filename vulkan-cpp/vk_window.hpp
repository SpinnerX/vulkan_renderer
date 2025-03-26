#pragma once
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vulkan-imports.hpp>

namespace vk {
    struct window_settings {
        int Width = -1;
        int Height = -1;
        std::string Name="";
    };

    class vk_window {
    public:
        vk_window(const std::string& p_tag, int p_width, int p_height);
        ~vk_window();

        void viewport_resize(int p_width, int p_height);

        uint32_t get_width() const { return m_settings.Width; }
        uint32_t get_height() const { return m_settings.Height; }

        std::string get_name() const { return m_settings.Name; }

        bool is_active() const { return !glfwWindowShouldClose(m_window_handler); }

        void create_window_surface(const VkInstance& p_instance);


        void center_window();

        void clean();


        static GLFWwindow* get_native_window() { return s_instance->m_window_handler; }

        operator GLFWwindow*() { return m_window_handler; }
        operator GLFWwindow*() const { return m_window_handler; }

        operator VkSurfaceKHR(){ return m_window_surface; }
        operator VkSurfaceKHR() const { return m_window_surface; }

    private:
        void on_create_window();

    private:
        static vk_window* s_instance;
        VkInstance m_instance = nullptr;
        window_settings m_settings{};

        GLFWwindow* m_window_handler=nullptr;
        VkSurfaceKHR m_window_surface = nullptr;
    };
};