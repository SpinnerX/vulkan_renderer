#include <GLFW/glfw3.h>
#include <shader_compiler/shader_compiler.hpp>
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
#include <vulkan-cpp/perspective_camera.hpp>
#include <renderer/mesh.hpp>
#include <wtr/watcher.hpp>
// #include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

/*

	[NOTE For Setting Up Multiple Render Passes]
		- These are the three main render passes I want to focus in try
   getting to work within the engine
		- As soon resizable swapchains could work and we can support
   viewports a lot better

	- Geometry Pass (First Renderpass)
		- Data contains crucial for framebuffers are COlor, Normals,
   depth, and other material-like for geometry

	Lighting Pass (Second RenderPass)
		- Sample textuyres (normals, depth) used from Geometry Pass
		- Performs lighting operations (using sampled data about light
   sources such as position, color, intensity, etc)
		- Send these information to a render target (swapchain image)
		[ Implementation High-level detail ]
			- Steps to doing this is the following
				1. Create and allocate anther vulkan image for
   each attachment
				2. Then associate an image view with it
				3. Then create a geometry render pass
   (VkRenderPass) that contains your geometry information (contained per vertex)
				4. Then create framebuffer that uses that
   specified renderpass

	Shadow Map
		- Involves casting a shadow onto your specific render targets
		- Contains information such as shadow casters, shadow biases
   information

		[ Implementation High-level detail ]
			- Creates depth target with (VkImage and VkImageView)
			- Create VkRenderPass (shadow pass object)
			- Create Shadow pass VkPipeline (graphics pipeline)
				- Would enable depth testing, backface culling
   (if needed)
			- Shadow Pass framebuffer
			- Per light perspective do the following:
				- Begin shadow render pass, bind framebuffer
				- Bind shadow graphics pipeline
				- Bind viewport and scissor
				- Bind vertex and index buffers of scene
   geometry
				- Bind descriptors (containing to its respective
   data)
				- End shadow renderpass


*/

class interactive_camera {
public:
    interactive_camera(float p_aspect_ratio)
      : m_aspect_ratio(p_aspect_ratio) {}

    void update() {
	glm::mat4 cameraRotation = get_rotation();
	m_position +=
	  glm::vec3(cameraRotation * glm::vec4(m_velocity * 0.5f, 0.f));
    }

    glm::mat4 get_rotation() {
	glm::quat pitchRotation =
	  glm::angleAxis(m_pitch, glm::vec3{ 1.f, 0.f, 0.f });
	glm::quat yawRotation =
	  glm::angleAxis(m_yaw, glm::vec3{ 0.f, -1.f, 0.f });

	return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }

    glm::mat4 get_view() {
	glm::mat4 cameraTranslation =
	  glm::translate(glm::mat4(1.f), m_position);
	glm::mat4 cameraRotation = get_rotation();
	return glm::inverse(cameraTranslation * cameraRotation);
    }

    glm::mat4 get_projection() {
	m_projection = glm::mat4(1.f);
	m_projection =
	  glm::perspective(glm::radians(70.f), m_aspect_ratio, 1000.f, 0.1f);
	// invert the Y direction on projection matrix so that we are more
	// similar to opengl and gltf axis
	m_projection[1][1] *= -1;
	return m_projection;
    }

public:
    glm::vec3 m_velocity = { 0.f, 0.f, 0.f };

private:
    float m_aspect_ratio = 0.f;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    glm::vec3 m_position = { 0.f, 0.f, 0.f };
    // vertical rotation
    float m_pitch{ 0.f };
    // horizontal rotation
    float m_yaw{ 0.f };
};

vk::vk_shader
test_shader_compilation() {
    vk::vk_shader test_compiled_shader = vk::vk_shader::from_source_files(
      "shaders/shader.vert", "shaders/shader.frag");

    // setup vertex attributes or smth
    // set some uniforms as well

    return test_compiled_shader;
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

    // vk::shader_watcher watcher("shaders");
    vk::vk_shader test_shader =
      vk::vk_shader("shaders/shader.vert", "shaders/shader.frag");
    // watcher.add_to_watchlist(test_shader, "shader.vert", "shader.frag");

    // vk::vk_shader test_shader =
    // vk::vk_shader("shader_useful_directory/geometry/vert.spv","shader_useful_directory/geometry/frag.spv");
    test_shader.set_vertex_attributes(
      { { .location = 0,
	  .binding = 0,
	  .format = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset = offsetof(vk::vertex, Position) },
	{ .location = 1,
	  .binding = 0,
	  .format = VK_FORMAT_R32G32B32A32_SFLOAT,
	  .offset = offsetof(vk::vertex, Color) },
	{ .location = 2,
	  .binding = 0,
	  .format = VK_FORMAT_R32G32B32A32_SFLOAT,
	  .offset = offsetof(vk::vertex, Normals) },
	{ .location = 3,
	  .binding = 0,
	  .format = VK_FORMAT_R32G32_SFLOAT,
	  .offset = offsetof(vk::vertex, Uv) } });

    test_shader.set_vertex_bind_attributes(
      { { .binding = 0,
	  .stride = sizeof(vk::vertex),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX } });

    // adding descriptor sets
    // creating our vertex and index buffers
    // vk::mesh new_mesh = load("models/Ball OBJ.obj");
    // vk::mesh new_mesh = load("models/viking_room.obj");
    vk::mesh new_mesh = vk::mesh("models/viking_room.obj");
    vk::vk_vertex_buffer test_vertex_buffer = new_mesh.get_vertex();

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
    // - Used to specify what kinds of data will this descriptor set be
    // containing
    vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(
      image_count,
      { { .binding = 0,
	  .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	  .descriptorCount = 1,
	  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
	  .pImmutableSamplers = nullptr },
	{ .binding = 1,
	  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	  .descriptorCount = 1,
	  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	  .pImmutableSamplers = nullptr } });

    // Vulkan Pipeline Specifications
    // specifically binding descriptions for the pipeline to use
    std::vector<vk::vertex_binding_description> binding_descriptions = {
	{ "Vertex", 0, sizeof(vk::vertex), VK_VERTEX_INPUT_RATE_VERTEX }
    };

    // Specify vertex attributes
    std::vector<vk::pipeline_vertex_attributes> vertex_attributes = {
	{ "inPosition",
	  0,
	  0,
	  offsetof(vk::vertex, Position),
	  VK_FORMAT_R32G32B32A32_SFLOAT },
	{ "inColor",
	  0,
	  1,
	  offsetof(vk::vertex, Color),
	  VK_FORMAT_R32G32B32A32_SFLOAT },
	{ "inNormals",
	  0,
	  2,
	  offsetof(vk::vertex, Normals),
	  VK_FORMAT_R32G32B32A32_SFLOAT },
	{ "inTexCoords",
	  0,
	  3,
	  offsetof(vk::vertex, Uv),
	  VK_FORMAT_R32G32B32A32_SFLOAT }
    };

    // setting up vulkan pipeline
    vk::vk_pipeline test_pipeline =
      vk::vk_pipeline(main_window_swapchain.get_renderpass(),
		      test_shader,
		      test_descriptor_sets.get_layout());

    // Loading and using textures
    // vk::vk_texture test_texture("models/viking_room.png");
    new_mesh.set_texture(0, "models/viking_room.png");
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
    vk::vk_texture my_texture = new_mesh.get_texture(0);

    test_descriptor_sets.update_test_descriptors(
      test_uniforms, test_vertex_buffer, my_texture);
    // test_descriptor_sets.update_test_descriptors(test_uniforms,
    // test_vertex_buffer, new_mesh.get_textures());

    /*

	// Essentially there are going to be vk_descriptor_set that is to be
    defined as a single descriptor set
	// Then set the descriptor set as probably a shader source group
    vk::vk_descriptor_set_manager desc_manager(image_size); // img_size = 3
    std::vector<vk::vk_descriptor_set> descriptor_sets(3);

      for(size_t i = 0; i < image_size; i++) {
	// desc_manager.write(i, uniform);
	descriptor_sets[i].write_uniform(uniform);
	descriptor_sets[i].write_texture(&test_texture);

      }

    */
    glm::vec3 Position = { 0.f, 0.f, 0.f };

    // perspective_camera camera = perspective_camera((float)width / height);
    // interactive_camera camera = interactive_camera(((width) / height));

    vk::vk_imgui test_imgui = vk::vk_imgui();
    VkRenderPass rp = main_window_swapchain.get_renderpass();
    test_imgui.initialize(initiating_vulkan,
			  main_physical_device,
			  main_window,
			  main_window_swapchain.image_size(),
			  main_window_swapchain.data().SurfaceFormat,
              main_window_swapchain);

    bool do_reload = false;

    auto watcher = wtr::watch("./shaders", [&](wtr::event ev) mutable {
	if (ev.effect_type == wtr::event::effect_type::modify &&
	    ev.path_type == wtr::event::path_type::file) {

	    if (ev.path_name.filename().string() == "shader.vert" ||
		ev.path_name.filename().string() == "shader.frag") {
		console_log_info(
		  "Detected shader reload stuff!!!!!!!!!!!!!!1l");
		do_reload = true;
	    }
	}
    });

    while (main_window.is_active()) {
	float dt = (float)glfwGetTime();

	if (do_reload ||
	    glfwGetKey(main_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
	      glfwGetKey(main_window, GLFW_KEY_R) == GLFW_PRESS) {
	    console_log_info("Reloading shaders...");

	    test_shader.compile("shaders/shader.vert", "shaders/shader.frag");
	    test_pipeline.reload_from_shader(
	      test_shader, rp, test_descriptor_sets.get_layout());

	    do_reload = false;
	}

	// if(glfwGetKey(main_window, GLFW_KEY_W) == GLFW_PRESS) { // forward
	//     camera.m_velocity.z = -1;

	// }
	// if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS) { // left
	//     camera.m_velocity.x = 1;
	// }
	// if(glfwGetKey(main_window, GLFW_KEY_S) == GLFW_PRESS) { // back
	//     camera.m_velocity.z = 1;
	// }
	// if(glfwGetKey(main_window, GLFW_KEY_D) == GLFW_PRESS) { // right
	//     camera.m_velocity.x = 1;
	// }

	// camera.update();

	// acquire next image ( then record)
	uint32_t frame = main_window_swapchain.read_acquired_frame();
	vk::vk_command_buffer current =
	  main_window_swapchain.get_active_command_buffer(frame);

	//! @note Updating our uniforms before we draw
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(
		       currentTime - startTime)
		       .count();

	camera_data_uniform ubo{};
	ubo.Model = glm::translate(ubo.Model, glm::vec3(0.f, 0.f, 0.f));
	ubo.Model = glm::rotate(glm::mat4(1.0f),
				time * glm::radians(90.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
			       glm::vec3(0.0f, 0.0f, 0.0f),
			       glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.Projection = glm::perspective(
	  glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
	ubo.Projection[1][1] *= -1;

	glm::mat4 MVP = ubo.Projection * ubo.View * ubo.Model;
	test_uniforms[frame].update(&MVP, sizeof(MVP));

	// Start recording
	main_window_swapchain.begin(current);


	test_pipeline.bind(current);
	test_descriptor_sets.bind(current,
				  main_window_swapchain.current_frame(),
				  test_pipeline.get_layout());

	// draw (after recording)
	new_mesh.draw(current);

	main_window_swapchain.end(current);
        

	//! @note This submits the command buffer and also presents the command
	//! buffer as well
	main_window_swapchain.submit(current);

    test_imgui.begin();
    ImGui::Begin("Viewport");
    ImGui::Button("Texture Image 0");

    ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
    // ImGui::Image(test_descriptor_sets.get(frame), ImVec2{ 100, 100 });

    // ImGui::Image()
    float floaty;
    if (ImGui::InputFloat("floaty", &floaty)) {
        console_log_info("Floaty is: {}", floaty);
    }
    ImGui::End();
    test_imgui.end(current, main_window_swapchain);


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

    //! @note This would probably be something specified in something like
    //! vk_context::submit_resource_free([](){ m_command_buffers.destroy(); })
    //! @note Whenever something needs to be submitted and destroyed when the
    //! application shuts down
    test_imgui.destroy();

    my_texture.destroy();
    for (size_t i = 0; i < test_uniforms.size(); i++) {
	test_uniforms[i].destroy();
    }

    test_descriptor_sets.destroy();
    // test_index_buffer.destroy();
    // test_vertex_buffer.destroy();
    new_mesh.destroy();
    test_pipeline.destroy();
    test_shader.destroy();
    main_window_swapchain.destroy();
    main_driver.destroy();
}
