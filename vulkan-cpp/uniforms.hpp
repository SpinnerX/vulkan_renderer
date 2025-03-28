#pragma once
#include <glm/glm.hpp>

// uniform data defined for our camera
struct camera_data_uniform {
    glm::mat4 ProjectionView{1.f};
    glm::mat4 Model{1.f};
};