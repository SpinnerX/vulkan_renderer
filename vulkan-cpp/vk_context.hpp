#pragma once
#include <string>
#include <vulkan/vulkan.hpp>
#include <functional>
#include <deque>

namespace vk {
    class vk_context {
    public:
        vk_context() = default;
        vk_context(const std::string& p_tag);
        ~vk_context();

        operator VkInstance() const { return m_instance; }
        operator VkInstance() { return m_instance; }

        static VkInstance current_context() { return *s_instance; }

        static void submit_resource_free(const std::function<void()>& p_callable);

        void cleanup();

    private:
        void resource_free(const std::function<void()>& p_callable);

    private:
        static vk_context* s_instance;
        std::deque<std::function<void()>> m_resource_to_free;
        VkInstance m_instance = nullptr;
    };
};