#include <vulkan/vulkan_core.h>
#include <fmt/core.h>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/vk_shader.hpp>
#include <vulkan-cpp/vk_pipeline.hpp>
#include <vulkan-cpp/vk_vertex_buffer.hpp>
#include <vulkan-cpp/vk_uniform_buffer.hpp>
#include <vulkan-cpp/uniforms.hpp>
#include <vulkan-cpp/vk_texture.hpp>
#include <renderer/first_person_camera.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// Default camera values
const float YAW = -90.0f;
// const float YAW = 45.0f;
const float PITCH = 0.0f;
// const float SPEED       =  2.5f;
// const float SENSITIVITY =  0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // constructor with vectors
    Camera(float p_aspect_ratio, glm::vec3 position = glm::vec3(0.0f, 0.f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 0.f, 0.0f),
            float yaw = YAW,
            float pitch = PITCH)
        : MovementSpeed(5.f)
        , MouseSensitivity(0.1f)
        , Zoom(ZOOM)
        , camera_mouse_sensitivity(0.1f) {
        Position = position;
        WorldUp = up;
        EulerRotation = { yaw, pitch, 1.f };
        AspectRatio = p_aspect_ratio;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt
    // Matrix
    glm::mat4 get_view() const { return View; }
    glm::mat4 get_projection() const { return Projection; }

    // processes input received from any keyboard-like input system. Accepts
    // input parameter in the form of camera defined ENUM (to abstract it
    // from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;

        if (direction == FORWARD)
            Position += get_front() * velocity;
        if (direction == BACKWARD)
            Position -= get_front() * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

        if (direction == UP) {
            Position += Up * velocity;
        }

        if (direction == DOWN) {
            Position -= Up * velocity;
        }

        // updateCameraVectors();
    }

    // processes input received from a mouse input system. Expects the
    // offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset,
                                float yoffset,
                                bool constrainPitch = true) {

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        EulerRotation.x += xoffset;
        EulerRotation.y += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get
        // flipped
        if (constrainPitch) {
            if (EulerRotation.y > 89.0f) {
                EulerRotation.y = 89.0f;
            }
            if (EulerRotation.y < -89.0f) {
                EulerRotation.y = -89.0f;
            }
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only
    // requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;

        if (Zoom < 1.0f) {
            Zoom = 1.0f;
        }

        if (Zoom > 45.0f) {
            Zoom = 45.0f;
        }
    }

    //! TODO: REMOVE THESE -- these should be user-defined
    void SetCameraMovementSpeed(float Sensitivity) {
        camera_movement_sensitivity = Sensitivity;
        MovementSpeed = camera_movement_sensitivity;
    }

    void SetCameraMouseSpeed(float Sensitivity) {
        camera_mouse_sensitivity = Sensitivity;
    }

    float GetCameraSensitivity() const { return camera_mouse_sensitivity; }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() {
        // calculate the new Front vector
        // glm::vec3 front;
        // front.x = cos(glm::radians(EulerRotation.x)) *
        // cos(glm::radians(EulerRotation.y)); front.y =
        // sin(glm::radians(EulerRotation.y)); front.z =
        // sin(glm::radians(EulerRotation.x)) *
        // cos(glm::radians(EulerRotation.y)); Front =
        // glm::normalize(front); also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(
            get_front(),
            WorldUp)); // normalize the vectors, because their length
                        // gets closer to 0 the more you look up or
                        // down which results in slower movement.
        Left = glm::normalize(glm::cross(-get_front(), WorldUp));
        Up = glm::normalize(glm::cross(Right, get_front()));
        Down = glm::normalize(glm::cross(-Right, WorldUp));
    }

public:
    void UpdateProjView() {
        //! TODO: Eventually we will have camera configurations that will
        //! utilize this.
        Projection =
            glm::perspective(45.0f, AspectRatio, .1f, 1000.f);
        View = glm::lookAt(Position, Position + get_front(), Up);
    }

    glm::vec3 get_front() const {
        glm::vec3 front_values;
        front_values.x = cos(glm::radians(EulerRotation.x)) *
                            cos(glm::radians(EulerRotation.y));
        front_values.y = sin(glm::radians(EulerRotation.y));
        front_values.z = sin(glm::radians(EulerRotation.x)) *
                            cos(glm::radians(EulerRotation.y));
        return glm::normalize(front_values);
    }

public:
    // camera Attributes
    glm::vec3 Position;
    // glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Down;
    glm::vec3 Right;
    glm::vec3 Left;
    glm::vec3 WorldUp;

    glm::mat4 Projection;
    glm::mat4 View;

    float AspectRatio = 0.f;

    // euler Angles
    // {x: Yaw, y: Pitch, z: Roll}
    glm::vec3 EulerRotation;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // float camera_mouse_sensitivity = 0.1f;
    float camera_mouse_sensitivity = 2.5f;
    float camera_movement_sensitivity = 2.5f;

    // toggling between cameras and checking if our current
    bool IsMainCamera = false;
};

int main(){
    logger::console_log_manager::initialize_logger_manager();

    //! @note Initializing GLFW
    if(!glfwInit()){
        fmt::println("glfwInit Initialized!!!");
        return -1;
    }

    //! @note Setup GLFW Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int width = 1600;
    int height = 900;

    //! @note 0.) Initialize Vulkan
    // create_vulkan_instance();
    vk::vk_context initiating_vulkan = vk::vk_context("vulkan");
    
    //! @note 1.) Initialize GLFW Window
    vk::vk_window main_window = vk::vk_window("Vulkan Window", width, height);

    //! @note 2.) Initiates Vulkan Surface
    main_window.create_window_surface(initiating_vulkan);
    main_window.center_window();

    //! @note 3.) Initialize Vulkan physical and logical drivers
    vk::vk_physical_driver main_physical_device = vk::vk_physical_driver(initiating_vulkan);
    vk::vk_driver main_driver = vk::vk_driver(main_physical_device);

    //! @note 4.) Initializing Swapchain
    vk::vk_swapchain main_window_swapchain = vk::vk_swapchain(main_physical_device, main_driver, main_window);
    main_window_swapchain.set_background_color({0.f, 1.f, 0.f, 1.f});

    vk::vk_shader test_shader = vk::vk_shader("shaders/vert.spv", "shaders/frag.spv");

    // adding descriptor sets
    std::vector<vk::vertex> vertices = {
        vk::vertex({-1.0f, -1.0f, 1.0f},  {0.0f, 0.0f}),	// top left
        vk::vertex({1.0f, -1.0f, 1.0f},   {0.0f, 1.0f}),	// top right
        vk::vertex({0.0f,  1.0f, 1.0f},   {1.0f, 1.0f}) 	// bottom middle
    };

    // std::vector<vk::vertex> vertices = {
    //     vk::vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),	// Bottom left
    //     vk::vertex({-1.0f, 1.0f, 0.0f},  {0.0f, 1.0f}), // Top left
    //     vk::vertex({1.0f,  1.0f, 0.0f},  {1.0f, 1.0f}), // Top right
    //     vk::vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // Bottom left
    //     vk::vertex({1.0f, 1.0f, 0.0f},   {1.0f, 1.0f}), // Top right
    //     vk::vertex({1.0f,  -1.0f, 0.0f}, {1.0f, 0.0f})  // Bottom right
    // };

    // creating our uniform buffer
    // vk::vk_vertex_buffer test_vertex_buffer = vk::vk_vertex_buffer(vertices);
    vk::vk_vertex_buffer test_vertex_buffer = vk::vk_vertex_buffer(vertices);

    // creating uniforms
    // vk::vk_uniform_buffer test_uniforms = vk::vk_uniform_buffer(main_window_swapchain.image_size(), sizeof(camera_data_uniform));
    std::vector<vk::vk_uniform_buffer> test_uniforms;
    test_uniforms.resize(main_window_swapchain.image_size());

    for(size_t i = 0; i < test_uniforms.size(); i++) {
        test_uniforms[i] = vk::vk_uniform_buffer(sizeof(camera_data_uniform));
    }

    // creating/Loading and using textures
    vk::vk_texture test_texture("textures/bricks.jpg");

    // creating descriptor sets

    /*
        Refactor descriptor sets

        - Set a single descriptor sets as the size of the amount of images
        - Then we have a single descriptor sets that we will create for scene objects (like textures, etc)
    
    */

    // vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(main_window_swapchain.image_size(), test_uniforms, sizeof(camera_data_uniform), &test_texture);
    // vk::vk_descriptor_set_properties properties = {
    //     "",
    //     0,
    //     vk::descriptor_type::STORAGE_BUFFER,
    //     vk::shader_stage::VERTEX
    // };

    //! @note Now without needing to manually set the layout bindings manually, this will set up the descriptor sets automatically
    std::vector<vk::vk_descriptor_set_properties> layouts = {
        {"in_Vertices", 0, vk::descriptor_type::STORAGE_BUFFER, vk::shader_stage::VERTEX},
        {"ubo", 1, vk::descriptor_type::UNIFORM_BUFFER, vk::shader_stage::VERTEX},
        {"texSampler", 2, vk::descriptor_type::IMAGE_AND_SAMPLER, vk::shader_stage::FRAGMENT}
    };
    vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(main_window_swapchain.image_size(), layouts);


    // std::array<vk::vk_shader, 2> shader_stages = {};
    vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window, main_window_swapchain.get_renderpass(), test_shader, test_descriptor_sets);

    // updating descriptor sets
    test_descriptor_sets.update_descriptor_sets(test_vertex_buffer, test_uniforms, &test_texture);

    // setting default perspective camera
    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;

    Camera perspective_camera((float)width / height);
    perspective_camera.IsMainCamera = true;



    // recording clear colors for all swapchain command buffers
    main_window_swapchain.record([&main_window_swapchain, &test_pipeline, &test_vertex_buffer, &test_descriptor_sets](const VkCommandBuffer& p_command_buffer){
        test_pipeline.bind(p_command_buffer);
        if(test_descriptor_sets.descriptors_count() > 0) {
            VkDescriptorSet desc_set = test_descriptor_sets.get(main_window_swapchain.current_frame());
            vkCmdBindDescriptorSets(p_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, test_pipeline.get_layout(), 0, 1, &desc_set, 0, nullptr);
        }

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(main_window_swapchain.get_extent().width),
            .height = static_cast<float>(main_window_swapchain.get_extent().height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(p_command_buffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = main_window_swapchain.get_extent(),
        };

        vkCmdSetScissor(p_command_buffer, 0, 1, &scissor);
        
        // uint32_t vertex_count = 3;
        // // uint32_t vertex_count = 6;
        // uint32_t instance_count = 1;
        // uint32_t first_vertex = 0;
        // uint32_t first_instance = 0;

        test_vertex_buffer.bind(p_command_buffer);
        test_vertex_buffer.draw(p_command_buffer);
        // vkCmdDraw(p_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
    });

    while(main_window.is_active()){
        float dt = (float)glfwGetTime();

        // acquire next image ( then record)
        perspective_camera.UpdateProjView();


        // draw (after recording)

        //! TODO: Could be relocated. All this needs to know is the current frame to update the uniforms
        main_window_swapchain.update_uniforms([&test_uniforms, &main_window, &perspective_camera, dt](const uint32_t& p_frame_index){
            static float angle = 0.0f;
            glm::mat4 rotation = glm::mat4(1.f);
            rotation = glm::rotate(rotation, glm::radians(angle), glm::normalize(glm::vec3(0.f, 0.f, 1.f)));
            angle += 0.001f;


            // if(glfwGetKey(main_window, GLFW_KEY_W) == GLFW_PRESS) {
            //     perspective_camera.ProcessKeyboard(FORWARD, dt);
            // }
            // if(glfwGetKey(main_window, GLFW_KEY_S) == GLFW_PRESS){
            //     perspective_camera.ProcessKeyboard(BACKWARD, dt);
            // }

            // if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS) {
            //     perspective_camera.ProcessKeyboard(LEFT, dt);
            // }

            // if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS) {
            //     perspective_camera.ProcessKeyboard(RIGHT, dt);
            // }

            // // UP
            // if(glfwGetKey(main_window, GLFW_KEY_Q) == GLFW_PRESS) {
            //     perspective_camera.ProcessKeyboard(UP, dt);
            // }

            // // DOWN
            // if(glfwGetKey(main_window, GLFW_KEY_E) == GLFW_PRESS) {
            //     perspective_camera.ProcessKeyboard(DOWN, dt);
            // }

            // glm::mat4 view_proj = perspective_camera.get_view() * perspective_camera.get_projection();
            glm::mat4 view_proj = perspective_camera.get_view() * perspective_camera.get_projection();

            glm::mat4 mvp = rotation;



            test_uniforms[p_frame_index].update(&mvp, sizeof(mvp));
        });

        // presenting frame (after drawing that frame)
        main_window_swapchain.present();
        
        glfwPollEvents();
    }

    // Lets make sure we destroy these objects in the order they're created

    // tell device to wait before destroying everything
    // doing this to ensure that we destroy them after everrythings done executing
    // needed to be called to ensure all children objects are executed just before they get destroyed!!
    vkDeviceWaitIdle(main_driver);

    test_texture.destroy();
    
    // test_uniforms.destroy();
    for(size_t i = 0; i < test_uniforms.size(); i++) {
        test_uniforms[i].destroy();
    }

    test_descriptor_sets.destroy();
    test_vertex_buffer.destroy();
    test_pipeline.destroy();
    test_shader.destroy();
    main_window_swapchain.destroy();
    main_driver.destroy();

}