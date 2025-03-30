#pragma once
#include <string>
#include <cstdint>

class window {
public:
    window(const std::string& p_name, uint32_t p_width, uint32_t p_height);

    static window* get_window() { return s_instance; }

private:
    static window* s_instance;
};