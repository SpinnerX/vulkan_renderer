#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct camera_properties {
    float Fov;
    float ZNear;
    float ZFar;
    float Width;
    float Height;
};

class first_person_camera {
public:
    static constexpr bool camera_orientation_left_hand = true;
    first_person_camera(float p_aspect_ratio, const glm::vec3& p_pos, const glm::vec3& p_target, const glm::vec3& p_up, const camera_properties& p_properties);

    glm::mat4 projection();

    glm::vec3 position();

    glm::mat4 view_mat();
    glm::mat4 view_proj_mat();

private:
    float m_aspect_ratio=0.f;
    glm::mat4 m_projection{0.f};
    glm::mat4 m_view{0.f};
    glm::mat4 m_view_proj{0.f};

    glm::vec3 m_position{0.f};
    glm::quat m_orientation{glm::vec3(0.f)};
};