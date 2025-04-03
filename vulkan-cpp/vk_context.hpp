#pragma once
#include <string>
#include <vulkan/vulkan.hpp>

namespace vk {
    class vk_context {
    public:
        vk_context() = default;
        vk_context(const std::string& p_tag);
        ~vk_context();

        operator VkInstance() const { return m_instance; }
        operator VkInstance() { return m_instance; }

        static VkInstance current_context() { return *s_instance; }

        void cleanup();

    private:
        static vk_context* s_instance;
        VkInstance m_instance = nullptr;
    };
};