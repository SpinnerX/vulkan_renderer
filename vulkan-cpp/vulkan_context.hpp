#pragma once
#include <vulkan/vulkan.h>

namespace vk{
    class context{
    public:
        context();

        static VkInstance get_instance() { return s_Instance->m_instance; }

        operator VkInstance(){ return m_instance; }

    private:
        VkInstance m_instance;
        static context* s_Instance;
    };
};