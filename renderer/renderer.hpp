#pragma once
#include <string>
#include <deque>
#include <functional>
#include <vulkan/vulkan.h>

class renderer {
public:
    renderer(const std::string& p_tag);

    template<typename UCallable>
    static void submit(const UCallable& p_callable) {
        
    }

private:
    std::deque<std::function<void(const VkCommandBuffer&)>> m_tasks;
    static renderer* s_instance;
};