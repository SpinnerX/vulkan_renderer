#pragma once
#include <glm/glm.hpp>

// uniform data defined for our camera
struct camera_data_uniform {
    glm::mat4 Model{ 1.f };
    glm::mat4 Projection{ 1.f };
    glm::mat4 View{ 1.f };
    float timer=0.f;
};

struct combined_uniforms {
    glm::mat4 m_model;
    float delta_time;
    glm::vec2 mouse_pos;
};