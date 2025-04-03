#include <renderer/first_person_camera.hpp>

first_person_camera::first_person_camera(
  float p_aspect_ratio,
  const glm::vec3& p_pos,
  const glm::vec3& p_target,
  const glm::vec3& p_up,
  const camera_properties& p_properties) {
    m_position = p_pos;
    m_aspect_ratio = p_aspect_ratio;

    if (camera_orientation_left_hand) {
        m_orientation = glm::lookAtLH(m_position, p_target, p_up);
        m_projection = glm::perspectiveLH(p_properties.Fov,
                                          m_aspect_ratio,
                                          p_properties.ZNear,
                                          p_properties.ZFar);
    }
    else {
        m_orientation = glm::lookAtRH(m_position, p_target, p_up);
        m_projection = glm::perspectiveRH(p_properties.Fov,
                                          m_aspect_ratio,
                                          p_properties.ZNear,
                                          p_properties.ZFar);
    }
}

glm::mat4
first_person_camera::projection() {
    return glm::mat4(1.f);
}

glm::vec3
first_person_camera::position() {
    return m_position;
}

glm::mat4
first_person_camera::view_mat() {
    glm::mat4 t = glm::translate(glm::mat4(1.f), -m_position);
    glm::mat4 r = glm::mat4_cast(m_orientation);
    glm::mat4 view = r * t;
    return view;
}

glm::mat4
first_person_camera::view_proj_mat() {
    glm::mat4 view = view_mat();
    glm::mat4 vp = m_projection * view;

    return vp;
}