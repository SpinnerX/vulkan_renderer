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
    template<>
    struct hash<vk::vertex> {
        size_t operator()(vk::vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec4>()(vertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.Uv) << 1);
        }
    };
}


// operator defer_""(){}

vk::mesh
load(const std::string& p_filename) {
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

            // if (index.vertex_index >= 0) {
            //     vertex.Position = {
            //         attrib.vertices[3 * index.vertex_index + 0],
            //         attrib.vertices[3 * index.vertex_index + 1],
            //         attrib.vertices[3 * index.vertex_index + 2]
            //     };

            //     // vertex.Color = { attrib.colors[3 * index.vertex_index + 0],
            //     //                  attrib.colors[3 * index.vertex_index + 1],
            //     //                  attrib.colors[3 * index.vertex_index + 2] };
            // }
			vertex.Position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};


			vertex.Color = {1.0f, 1.0f, 1.0f, 1.f};

            // if (index.normal_index >= 0) {
            //     vertex.Normals = {
            //         attrib.normals[3 * index.normal_index + 0],
            //         attrib.normals[3 * index.normal_index + 1],
            //         attrib.normals[3 * index.normal_index + 2]
            //     };
            // }
			vertex.Uv = { attrib.texcoords[2 * index.texcoord_index + 0],attrib.texcoords[2 * index.texcoord_index + 1] };

            // if (index.texcoord_index >= 0) {
            //     vertex.Uv = { attrib.texcoords[2 * index.texcoord_index + 0],attrib.texcoords[2 * index.texcoord_index + 1] };
            // }

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

int
main() {
    logger::console_log_manager::initialize_logger_manager();

    //! @note Initializing GLFW
    if (!glfwInit()) {
        fmt::println("glfwInit Initialized!!!");
        return -1;
    }

    //! @note Setup GLFW Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int width = 900;
    int height = 600;

    //! @note 0.) Initialize Vulkan
    // create_vulkan_instance();
    vk::vk_context initiating_vulkan = vk::vk_context("vulkan");

    //! @note 1.) Initialize GLFW Window
    vk::vk_window main_window = vk::vk_window("Vulkan Window", width, height);

    //! @note 2.) Initiates Vulkan Surface
    main_window.create_window_surface(initiating_vulkan);
    main_window.center_window();

    //! @note 3.) Initialize Vulkan physical and logical drivers
    vk::vk_physical_driver main_physical_device =
      vk::vk_physical_driver(initiating_vulkan);
    vk::vk_driver main_driver = vk::vk_driver(main_physical_device);

    //! @note 4.) Initializing Swapchain
    vk::vk_swapchain main_window_swapchain =
      vk::vk_swapchain(main_physical_device, main_driver, main_window);
    main_window_swapchain.set_background_color({ 0.f, 0.f, 0.f, 1.f });

    vk::vk_shader test_shader = vk::vk_shader("shaders/vert.spv", "shaders/frag.spv");
	// vk::vk_shader test_shader = vk::vk_shader("shader_useful_directory/geometry/vert.spv","shader_useful_directory/geometry/frag.spv");
    test_shader.set_vertex_attributes({
		{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, Position)},
		{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vk::vertex, Color)},
		{.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(vk::vertex, Uv)}
    });

	test_shader.set_vertex_bind_attributes({
		{.binding = 0, .stride = sizeof(vk::vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}
	});

    // adding descriptor sets
    // creating our vertex and index buffers
    // vk::mesh new_mesh = load("models/Ball OBJ.obj");
    vk::mesh new_mesh = load("models/viking_room.obj");
    vk::vk_vertex_buffer test_vertex_buffer = new_mesh.get_vertex();
    vk::vk_index_buffer test_index_buffer = new_mesh.get_index();

    uint32_t size_of_bytes = sizeof(camera_data_uniform);

    // creating uniforms
    std::vector<vk::vk_uniform_buffer> test_uniforms;
    test_uniforms.resize(main_window_swapchain.image_size());

    for (size_t i = 0; i < test_uniforms.size(); i++) {
        test_uniforms[i] = vk::vk_uniform_buffer(size_of_bytes);
    }

    /*
        Refactor descriptor sets

        - Set a single descriptor sets as the size of the amount of images
        - Then we have a single descriptor sets that we will create for scene
       objects (like textures, etc)
    */

    uint32_t image_count = main_window_swapchain.image_size();

    //! @note Now without needing to manually set the layout bindings manually,
    //! this will set up the descriptor sets automatically
    // this descriptor set layout is for shaders/shader.*
    /*
	std::vector<vk::vk_descriptor_set_properties> descriptor_layouts = {
        { "in_Vertices", 0, vk::descriptor_type::STORAGE_BUFFER, vk::shader_stage::VERTEX },
        { "ubo", 1, vk::descriptor_type::UNIFORM_BUFFER, vk::shader_stage::VERTEX },
        { "texSampler", 2, vk::descriptor_type::IMAGE_AND_SAMPLER, vk::shader_stage::FRAGMENT }
    };

    vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(image_count, descriptor_layouts);
	*/
	
	/*
	
		p_layouts
		- Used to specify what kinds of data will this descriptor set be containing

	*/
	vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(image_count,
	{
		{.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .pImmutableSamplers  = nullptr},
		{.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers  = nullptr}
	}
	);

    // Vulkan Pipeline Specifications
    // specifically binding descriptions for pipeline
    std::vector<vk::vertex_binding_description> binding_descriptions = {
        { "Vertex", 0, sizeof(vk::vertex), VK_VERTEX_INPUT_RATE_VERTEX }
    };

    // specifically vertex attributes
    std::vector<vk::pipeline_vertex_attributes> vertex_attributes = {
        { "inPosition", 0, 0, offsetof(vk::vertex, Position), VK_FORMAT_R32G32B32A32_SFLOAT },
        { "inColor", 0, 1, offsetof(vk::vertex, Color), VK_FORMAT_R32G32B32A32_SFLOAT },
        { "inTexCoords", 0, 2, offsetof(vk::vertex, Uv), VK_FORMAT_R32G32B32A32_SFLOAT }
    };

    // setting up vulkan pipeline
    vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window_swapchain.get_renderpass(),test_shader, test_descriptor_sets.get_layout());

    // Loading and using textures
    vk::vk_texture test_texture("models/viking_room.png");
    // vk::vk_texture test_texture("textures/bricks.jpg");

    // updating descriptor sets
    /*
        API For writing uniforms to the shader
            - mesh contains index and vertex buffers
            - test_uniforms passes all of our camera data
            - passing in our texture
        update_descriptor_sets(mesh, test_uniforms)

        This call should be updated to doing this:

        Using vk_descriptor_set used as a single descriptor set rather then
       supporting multiple. That is something that the vk_descriptor_set_manager
       should do

        descriptor_set[i].update_descriptor_set(uniform_buffer[i]);
    */

    // test_descriptor_sets.update_uniforms(test_uniforms);
    // test_descriptor_sets.update_texture(&test_texture);
    // test_descriptor_sets.update_vertex(test_vertex_buffer);

	test_descriptor_sets.update_test_descriptors(test_uniforms, test_vertex_buffer, test_texture);

    /*

	// Essentially there are going to be vk_descriptor_set that is to be defined as a single descriptor set
	// Then set the descriptor set as probably a shader source group
    vk::vk_descriptor_set_manager desc_manager(image_size); // img_size = 3
    std::vector<vk::vk_descriptor_set> descriptor_sets(3);

      for(size_t i = 0; i < image_size; i++) {
        // desc_manager.write(i, uniform);
        descriptor_sets[i].write_uniform(uniform);
        descriptor_sets[i].write_texture(&test_texture);

      }

    */

    // recording clear colors for all swapchain command buffers
    main_window_swapchain.record([&main_window_swapchain, &test_pipeline, &test_vertex_buffer, &test_index_buffer, &test_descriptor_sets](const VkCommandBuffer& p_command_buffer) {
          test_pipeline.bind(p_command_buffer);

          test_descriptor_sets.bind(p_command_buffer,
                                    main_window_swapchain.current_frame(),
                                    test_pipeline.get_layout());

          test_vertex_buffer.bind(p_command_buffer);
          test_index_buffer.bind(p_command_buffer);

          if (test_index_buffer.has_indices()) {
              test_index_buffer.draw(p_command_buffer);
          }
          else {
              test_vertex_buffer.draw(p_command_buffer);
          }
	});

	glm::vec3 Position = {0.f, 0.f, 0.f};

    // vk::vk_imgui test_imgui = vk::vk_imgui();
    // VkRenderPass rp = main_window_swapchain.get_renderpass();
    // test_imgui.initialize(initiating_vulkan, main_physical_device,
    // main_window_swapchain);

    while (main_window.is_active()) {
        float dt = (float)glfwGetTime();

        // acquire next image ( then record)
        // test_imgui.begin();

        // ImGui::Button("Test");
        // test_imgui.end();

        // draw (after recording)

		if(glfwGetKey(main_window, GLFW_KEY_W) == GLFW_PRESS) {
			Position.x += (0.1f) * dt;
		}
		if(glfwGetKey(main_window, GLFW_KEY_S) == GLFW_PRESS) {
			Position.x -= (0.1f) * dt;
		}
		if(glfwGetKey(main_window, GLFW_KEY_Q) == GLFW_PRESS) {
			Position.y += (0.1f) * dt;
		}
		if(glfwGetKey(main_window, GLFW_KEY_E) == GLFW_PRESS) {
			Position.y -= (0.1f) * dt;
		}

		if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS) {
			Position.z += (0.1f) * dt;
		}
		if(glfwGetKey(main_window, GLFW_KEY_D) == GLFW_PRESS) {
			Position.z -= (0.1f) * dt;
		}
		
        //! TODO: Could be relocated. All this needs to know is the current
        //! frame to update the uniforms
        main_window_swapchain.update_uniforms([&test_uniforms, &main_window, width, height, &Position](const uint32_t& p_frame_index) {
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			camera_data_uniform ubo{};
			// ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(1.f, 0.f, 1.0f));
			// ubo.View = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			// ubo.Projection = glm::perspective(glm::radians(45.0f), width / (float) height, 0.9f, 10.0f);
			// ubo.Projection[1][1] *= -1;
			ubo.Model = glm::mat4(1.f);
			ubo.Model = glm::translate(ubo.Model, Position);
			ubo.Model = glm::rotate(ubo.Model, time * glm::radians(90.0f), glm::vec3(5.0f, 5.0f, 5.0f));
			ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, -0.50f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.Projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 10.0f);
			ubo.Projection[1][1] *= -1;

			// test_uniforms[p_frame_index].update(&mvp, sizeof(mvp));
			test_uniforms[p_frame_index].update(&ubo, sizeof(ubo));
		});

        // presenting frame (after drawing that frame)
        main_window_swapchain.present();

        glfwPollEvents();
    }

    // Lets make sure we destroy these objects in the order they're created

    // tell device to wait before destroying everything
    // doing this to ensure that we destroy them after everrythings done
    // executing needed to be called to ensure all children objects are executed
    // just before they get destroyed!!
    vkDeviceWaitIdle(main_driver);

    test_texture.destroy();

    // test_uniforms.destroy();
    for (size_t i = 0; i < test_uniforms.size(); i++) {
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