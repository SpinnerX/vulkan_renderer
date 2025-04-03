#pragma once
#include <string>
#include <deque>
#include <functional>
#include <vulkan/vulkan.h>

class renderer {
public:
    renderer(const std::string& p_tag);

    // implications when its the start of the frame and the end of a frame
    void begin();

    void end();

    template<typename UCallable>
    static void submit(const UCallable& p_callable) {}

    template<typename UCallable>
    static void submit_free(const UCallable& p_callable) {}

private:
    static std::deque<std::function<void(const VkCommandBuffer&)>> s_tasks;
    static std::deque<std::function<void(VkCommandBuffer&)>> s_deletion_tasks;

    static renderer* s_instance;
};