#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
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
#include <vulkan-cpp/vk_descriptor_set.hpp>
#include <imgui.h>
#include <vulkan-cpp/vk_imgui.hpp>

#include <tiny_obj_loader.h>


namespace std {
    template<> struct hash<vk::vertex> {
        size_t operator()(vk::vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.Uv) << 1);
        }
    };
}

vk::mesh load(const std::string& p_filename) {
    vk::mesh return_mesh;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    //! @note If we return the constructor then we can check if the mesh
    //! loaded successfully
    //! @note We also receive hints if the loading is successful!
    //! @note Return default constructor automatically returns false means
    //! that mesh will return the boolean as false because it wasnt
    //! successful
    if (!tinyobj::LoadObj(
            &attrib, &shapes, &materials, &warn, &err, p_filename.c_str())) {
        console_log_warn("Could not load model from path {}", p_filename);
    }
    else {
        console_log_info("Model Loaded = {}", p_filename);
    }

    std::vector<vk::vertex> vertices;
    // std::vector<uint32_t> indices;
    std::vector<uint32_t> indices;
    std::unordered_map<vk::vertex, uint32_t> unique_vertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            vk::vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.Color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }

            if (index.normal_index >= 0) {
                vertex.Normals = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.Uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            // vertices.push_back(vertex);
            if (unique_vertices.contains(vertex) == 0) {
                unique_vertices[vertex] =
                    static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(unique_vertices[vertex]);
        }
    }

    return_mesh = vk::mesh(vertices, indices);
    return return_mesh;
}

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
    // vk::vk_shader test_shader = vk::vk_shader("shader_useful_directory/geometry/vert.spv", "shader_useful_directory/geometry/frag.spv");

    // adding descriptor sets
    // std::vector<vk::vertex> vertices = {
    //     vk::vertex({-1.0f, -1.0f, 1.0f},  {0.0f, 0.0f}),	// top left
    //     vk::vertex({1.0f, -1.0f, 1.0f},   {0.0f, 1.0f}),	// top right
    //     vk::vertex({0.0f,  1.0f, 1.0f},   {1.0f, 1.0f}) 	// bottom middle
    // };

    // std::vector<vk::vertex> vertices = {
    //     vk::vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),	// Bottom left
    //     vk::vertex({-1.0f, 1.0f, 0.0f},  {0.0f, 1.0f}), // Top left
    //     vk::vertex({1.0f,  1.0f, 0.0f},  {1.0f, 1.0f}), // Top right
    //     vk::vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // Bottom left
    //     vk::vertex({1.0f, 1.0f, 0.0f},   {1.0f, 1.0f}), // Top right
    //     vk::vertex({1.0f,  -1.0f, 0.0f}, {1.0f, 0.0f})  // Bottom right
    // };
    // std::vector<vk::vertex> vertices = {
    //     vk::vertex({1.0f, 0.0f, 0.0f}, {-0.5f, -0.5f}),
    //     vk::vertex({0.0f, 1.0f, 0.0f}, {0.5f, -0.5f}),
    //     vk::vertex({0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}),
    //     vk::vertex({1.0f, 1.0f, 1.0f}, {-0.5f, 0.5f})
    // };

    // // std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    // std::vector<uint32_t> indices = {
    //     0, 1, 2, 2, 3, 1,
    // };

    // creating our uniform buffer
    // vk::vk_vertex_buffer test_vertex_buffer = vk::vk_vertex_buffer(vertices);
    // vk::vk_index_buffer test_index_buffer = vk::vk_index_buffer(indices);



    // vk::vk_vertex_buffer test_vertex_buffer = vk::vk_vertex_buffer();
    // vk::vk_index_buffer test_index_buffer = vk::vk_index_buffer();

    vk::mesh new_mesh = load("models/viking_room.obj");
    vk::vk_vertex_buffer test_vertex_buffer = new_mesh.get_vertex();
    vk::vk_index_buffer test_index_buffer = new_mesh.get_index();

    // creating uniforms
    std::vector<vk::vk_uniform_buffer> test_uniforms;
    test_uniforms.resize(main_window_swapchain.image_size());

    for(size_t i = 0; i < test_uniforms.size(); i++) {
        test_uniforms[i] = vk::vk_uniform_buffer(sizeof(camera_data_uniform));
    }

    /*
        Refactor descriptor sets

        - Set a single descriptor sets as the size of the amount of images
        - Then we have a single descriptor sets that we will create for scene objects (like textures, etc)
    */

    uint32_t image_count = main_window_swapchain.image_size();

    //! @note Now without needing to manually set the layout bindings manually, this will set up the descriptor sets automatically
    // this descriptor set layout is for shaders/shader.*
    std::vector<vk::vk_descriptor_set_properties> layouts = {
        {"in_Vertices", 0, vk::descriptor_type::STORAGE_BUFFER, vk::shader_stage::VERTEX},
        {"ubo", 1, vk::descriptor_type::UNIFORM_BUFFER, vk::shader_stage::VERTEX},
        {"texSampler", 2, vk::descriptor_type::IMAGE_AND_SAMPLER, vk::shader_stage::FRAGMENT}
    };

    // This is descriptors for shader_useful_directory/geometry/shader.* shaders
    // std::vector<vk::vk_descriptor_set_properties> layouts = {
    //     {"inPosition", 0, vk::descriptor_type::UNIFORM_BUFFER, vk::shader_stage::VERTEX},
    //     {"inPosition", 1, vk::descriptor_type::IMAGE_AND_SAMPLER, vk::shader_stage::FRAGMENT},
    // };

    vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(image_count, layouts);
    // vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(main_window_swapchain.image_size(), test_uniforms);
    std::vector<vk::pipeline_vertex_attributes> vertex_attributes = {
        {.Name = "isPos", .Binding = 0, .Location = 0, .Format = VK_FORMAT_R32G32B32_SFLOAT, .Offset = offsetof(vk::vertex, Position)}
    };

    // setting up vulkan pipeline
    vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window_swapchain.get_renderpass(), test_shader, test_descriptor_sets.get_layout(), {});

    // vk::vk_pipeline test_pipeline2 = vk::vk_pipeline(main_window_swapchain.get_renderpass(), test_shader, test_descriptor_sets.get_layout());

    // Loading and using textures
    // vk::vk_texture test_texture("models/viking_room.png");
    vk::vk_texture test_texture("textures/bricks.jpg");

    // updating descriptor sets
    /*
        API For writing uniforms to the shader
            - mesh contains index and vertex buffers
            - test_uniforms passes all of our camera data
            - passing in our texture
        update_descriptor_sets(mesh, test_uniforms)
    */
    test_descriptor_sets.update_descriptor_sets(test_vertex_buffer, test_uniforms, &test_texture);



    // recording clear colors for all swapchain command buffers
    main_window_swapchain.record([&main_window_swapchain, &test_pipeline, &test_vertex_buffer, &test_index_buffer, &test_descriptor_sets](const VkCommandBuffer& p_command_buffer){
        test_pipeline.bind(p_command_buffer);

        test_descriptor_sets.bind(p_command_buffer, main_window_swapchain.current_frame(), test_pipeline.get_layout());


        test_vertex_buffer.bind(p_command_buffer);
        test_index_buffer.bind(p_command_buffer);

        // test_vertex_buffer.draw(p_command_buffer);

        if(test_index_buffer.has_indices()) {
            test_index_buffer.draw(p_command_buffer);   
        }
        else {
            test_vertex_buffer.draw(p_command_buffer);
        }
    });

    // vk::vk_imgui test_imgui = vk::vk_imgui();
    // VkRenderPass rp = main_window_swapchain.get_renderpass();
    // test_imgui.initialize(initiating_vulkan, main_physical_device, main_window_swapchain);

    while(main_window.is_active()){
        float dt = (float)glfwGetTime();

        // acquire next image ( then record)
        // test_imgui.begin();

        // ImGui::Button("Test");
        // test_imgui.end();

        // draw (after recording)

        //! TODO: Could be relocated. All this needs to know is the current frame to update the uniforms
        main_window_swapchain.update_uniforms([&test_uniforms, &main_window, dt](const uint32_t& p_frame_index){
            static float angle = 0.0f;
            glm::mat4 rotation = glm::mat4(1.f);
            rotation = glm::rotate(rotation, glm::radians(angle), glm::normalize(glm::vec3(0.f, 0.f, 1.f)));
            angle += 0.001f;

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
    test_index_buffer.destroy();
    test_vertex_buffer.destroy();
    test_pipeline.destroy();
    test_shader.destroy();
    main_window_swapchain.destroy();
    main_driver.destroy();

}