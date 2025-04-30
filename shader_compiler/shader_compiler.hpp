#pragma once
/* Just test*/

#include <string>

#include "vulkan-cpp/vk_descriptor_set.hpp"

namespace vk {

class shader_file_handle {
public:
    shader_file_handle(const std::string& p_filename, shader_stage p_stage);
    ~shader_file_handle();


};
}
