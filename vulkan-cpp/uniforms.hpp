#pragma once
#include <glm/glm.hpp>

// uniform data defined for our camera
struct camera_data_uniform {
    glm::mat4 ProjectionView{ 1.f };
    glm::mat4 Model{ 1.f };
};

struct camera_data_uniform2 {
    glm::mat4 Model{ 1.f };
    glm::mat4 Projection{ 1.f };
    glm::mat4 View{ 1.f };
};