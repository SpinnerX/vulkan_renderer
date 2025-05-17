#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <fmt/core.h>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
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
#include <renderer/widgets.hpp>
#include <renderer/camera.hpp>

/*

	[NOTE For Setting Up Multiple Render Passes]
		- These are the three main render passes I want to focus in try getting to work within the engine
		- As soon resizable swapchains could work and we can support viewports a lot better

	- Geometry Pass (First Renderpass)
		- Data contains crucial for framebuffers are COlor, Normals, depth, and other material-like for geometry
	
	Lighting Pass (Second RenderPass)
		- Sample textuyres (normals, depth) used from Geometry Pass
		- Performs lighting operations (using sampled data about light sources such as position, color, intensity, etc)
		- Send these information to a render target (swapchain image)
		[ Implementation High-level detail ]
		 	- Steps to doing this is the following
				1. Create and allocate anther vulkan image for each attachment
				2. Then associate an image view with it
				3. Then create a geometry render pass (VkRenderPass) that contains your geometry information (contained per vertex)
				4. Then create framebuffer that uses that specified renderpass
	
	Shadow Map
		- Involves casting a shadow onto your specific render targets
		- Contains information such as shadow casters, shadow biases information

		[ Implementation High-level detail ]
		 	- Creates depth target with (VkImage and VkImageView)
			- Create VkRenderPass (shadow pass object)
			- Create Shadow pass VkPipeline (graphics pipeline)
				- Would enable depth testing, backface culling (if needed)
			- Shadow Pass framebuffer
			- Per light perspective do the following:
				- Begin shadow render pass, bind framebuffer
				- Bind shadow graphics pipeline
				- Bind viewport and scissor
				- Bind vertex and index buffers of scene geometry
				- Bind descriptors (containing to its respective data)
				- End shadow renderpass

*/

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
    vk::vk_physical_driver main_physical_device =
      vk::vk_physical_driver(initiating_vulkan);
    vk::vk_driver main_driver = vk::vk_driver(main_physical_device);

    //! @note 4.) Initializing Swapchain
    vk::vk_swapchain main_window_swapchain =
      vk::vk_swapchain(main_physical_device, main_driver, main_window);
    main_window_swapchain.set_background_color({ 0.f, 0.f, 0.f, 1.f });
    
    //vk::shader_watcher watcher("shaders");
    // vk::vk_shader test_shader = vk::vk_shader("shaders/shader.vert", "shaders/shader.frag");

    vk::vk_shader_group group1 = {
        {"shaders/shader.vert", vk::shader_stage2::Vertex},
        {"shaders/shader.frag", vk::shader_stage2::Fragment},
    };

    // VkVertexInputAttributeDescription::location
    // VkVertexInputAttributeDescription
    group1.set_vertex_attributes({
		{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, Position)},
		{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vk::vertex, Color)},
        {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vk::vertex, Normals)},
		{.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(vk::vertex, Uv)}
    });
    group1.set_vertex_bind_attributes({
		{.binding = 0, .stride = sizeof(vk::vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}
	});
    // vk::vk_shader vert_shader = vk_shader("shaders/shader.vert", shader_stage::Vertex);
    // vk::vk_shader frag_shader = vk_shader("shaders/shader.frag", shader_stage::Fragment);


    // watcher.add_to_watchlist(test_shader, "shader.vert", "shader.frag");
    
	// vk::vk_shader test_shader = vk::vk_shader("shader_useful_directory/geometry/vert.spv","shader_useful_directory/geometry/frag.spv");
    // test_shader.set_vertex_attributes({
	// 	{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, Position)},
	// 	{.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vk::vertex, Color)},
    //     {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vk::vertex, Normals)},
	// 	{.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(vk::vertex, Uv)}
    // });

	// test_shader.set_vertex_bind_attributes({
	// 	{.binding = 0, .stride = sizeof(vk::vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}
	// });

    // adding descriptor sets
    // creating our vertex and index buffers
    // vk::mesh new_mesh = load("models/Ball OBJ.obj");
    // vk::mesh new_mesh = load("models/viking_room.obj");
    vk::mesh new_mesh = vk::mesh("models/viking_room.obj");
    vk::mesh sphere_mesh = vk::mesh("models/sphere.obj");

    uint32_t size_of_bytes = sizeof(combined_uniforms);

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
    // - Used to specify what kinds of data will this descriptor set be containing
	vk::vk_descriptor_set test_descriptor_sets = vk::vk_descriptor_set(image_count,
	{
		{.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, .pImmutableSamplers  = nullptr},
		{.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers  = nullptr}
	}
	);

    // Vulkan Pipeline Specifications
    // specifically binding descriptions for the pipeline to use
    std::vector<vk::vertex_binding_description> binding_descriptions = {
        { "Vertex", 0, sizeof(vk::vertex), VK_VERTEX_INPUT_RATE_VERTEX }
    };

    // Specify vertex attributes
    std::vector<vk::pipeline_vertex_attributes> vertex_attributes = {
        { "inPosition", 0, 0, offsetof(vk::vertex, Position), VK_FORMAT_R32G32B32A32_SFLOAT },
        { "inColor", 0, 1, offsetof(vk::vertex, Color), VK_FORMAT_R32G32B32A32_SFLOAT },
        { "inNormals", 0, 2, offsetof(vk::vertex, Normals), VK_FORMAT_R32G32B32A32_SFLOAT },
        { "inTexCoords", 0, 3, offsetof(vk::vertex, Uv), VK_FORMAT_R32G32B32A32_SFLOAT }
    };

    // setting up vulkan pipeline
    // vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window_swapchain.get_renderpass(),test_shader, test_descriptor_sets.get_layout());
    vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window_swapchain.get_renderpass(), group1, test_descriptor_sets.get_layout());

    // Loading and using textures
    new_mesh.set_texture(0, "models/viking_room.png");


    vk::vk_texture my_texture = new_mesh.get_texture(0);

    // test_descriptor_sets.update_test_descriptors(test_uniforms, test_vertex_buffer, my_texture);
    test_descriptor_sets.update_mesh(test_uniforms, new_mesh);
    // sphere_mesh.set_texture(1, "textures/bricks.jpg");
    // test_descriptor_sets.update_mesh(test_uniforms, sphere_mesh);

    /*
        Vulkan Descriptor Set Extended API


        vk_descriptor_set test_descriptor{
            // Sets up the descriptor set pool sizes that gets utilized by the descriptor set layouts
            { {UniformBuffer, .size = 1000}, {Image_Combined_Sampler, .size=1000}, }
        };
    */


	vk::vk_imgui test_imgui = vk::vk_imgui();
    VkRenderPass rp = main_window_swapchain.get_renderpass();
    test_imgui.initialize(initiating_vulkan, main_physical_device, rp, main_window_swapchain.image_size(), main_window_swapchain.data().SurfaceFormat);

    bool do_reload = false;

    auto watcher = wtr::watch("./shaders", [&](wtr::event ev) mutable {
        if (ev.effect_type == wtr::event::effect_type::modify && ev.path_type == wtr::event::path_type::file) {

            if (ev.path_name.filename().string() == "shader.vert" || ev.path_name.filename().string() == "shader.frag") {
                console_log_info("Detected shader reload stuff!!!!!!!!!!!!!!1l");
                do_reload = true;
            }
        }
    });

    camera test_camera(width/height);
    test_camera.Position = {-3.69f, -0.73f, -3.84f};
    static float g_delta_time = 0.f;
    float previous_time = 0.f;
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 scale = {1.f, 1.f, 1.f};
    glm::vec3 rotation = {1.50f, 8.70f, -0.10f};
    glm::vec4 color{1.f};
    // glm::highp_vec3 rotation = {1.f, 1.f, 1.f};

    while (main_window.is_active()) {
        float dt = (float)glfwGetTime();
        g_delta_time = (dt - previous_time);
        previous_time = dt;
        
        if (do_reload || glfwGetKey(main_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
            glfwGetKey(main_window, GLFW_KEY_R) == GLFW_PRESS) {
            console_log_info("Reloading shaders...");
            group1.compile();
            test_pipeline.reload_from_shader_sources(group1, rp, test_descriptor_sets.get_layout());
            do_reload = false;
            
        }

        // acquire next image ( then record)
        uint32_t frame = main_window_swapchain.read_acquired_frame();
        vk::vk_command_buffer current = main_window_swapchain.get_active_command_buffer(frame);

        //! @note Updating our uniforms before we draw
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        if(glfwGetKey(main_window, GLFW_KEY_W) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Forward, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_S) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Backward, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Left, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_D) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Right, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_Q) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Up, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_E) == GLFW_PRESS) {
            test_camera.process_keyboard(CameraMovement::Down, g_delta_time);
        }

        if(glfwGetKey(main_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            if(glfwGetMouseButton(main_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                double xPos, yPos;
                glfwGetCursorPos(main_window, &xPos, &yPos);
                float x_offset = (float)xPos;
                float velocity = x_offset * g_delta_time;

                test_camera.process_mouse_movement(-velocity, 0.f);
            }

            if(glfwGetMouseButton(main_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double xPos, yPos;
                glfwGetCursorPos(main_window, &xPos, &yPos);
                float y_offset = (float)yPos;
                float velocity = y_offset * g_delta_time;
                test_camera.process_mouse_movement(velocity, 0.f);
            }

            if(glfwGetMouseButton(main_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                double xPos, yPos;
                glfwGetCursorPos(main_window, &xPos, &yPos);
                float x_offset = (float)yPos;
                float velocity = x_offset * g_delta_time;
                test_camera.process_mouse_movement(0.f, velocity);
            }

            if(glfwGetKey(main_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                double xPos, yPos;
                glfwGetCursorPos(main_window, &xPos, &yPos);
                float y_offset = (float)yPos;
                float velocity = y_offset * g_delta_time;
                test_camera.process_mouse_movement(0.f, -velocity);
            }
        }

        camera_data_uniform ubo{};
        // ubo.Model = glm::translate(ubo.Model, glm::vec3(0.f, 0.f, 0.f));
        // ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.Projection = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
        // Rotation = {1.50f, 8.70f, -0.10f}
        ubo.Model = glm::translate(ubo.Model, position); // this is only for one object. There should be one model matrix per scene object (glm::mat4)
        // ubo.Model = glm::rotate(ubo.Model, glm::radians(45.0f), rotation);
        ubo.Model = glm::scale(ubo.Model, scale);
        glm::mat4 rotation_matrix = glm::mat4(glm::quat(rotation));
        ubo.Model *= rotation_matrix;
        ubo.View = test_camera.get_view();
        ubo.Projection = test_camera.get_projection();
        ubo.Projection[1][1] *= -1;
        ubo.timer = dt;

        glm::mat4 MVP = ubo.Projection * ubo.View * ubo.Model;
        glm::vec2 mouse_pos;
        double x, y;

        glfwGetCursorPos(main_window, &x, &y);
        mouse_pos.x = (float)x / 1600.0f;
        mouse_pos.y = (float)y / 900.0f;
        combined_uniforms uniforms = {
            .m_model = MVP,
            .delta_time = dt,
            .mouse_pos = mouse_pos
        };
        test_uniforms[frame].update(&uniforms, sizeof(uniforms));
        // test_uniforms[frame].update(&dt, sizeof(dt));
        // test_uniforms[frame].update(&color, sizeof(color));

        test_camera.update_proj_view();


        // Start recording
        main_window_swapchain.begin(current);
        test_imgui.begin();


        test_pipeline.bind(current);
        test_descriptor_sets.bind(current,main_window_swapchain.current_frame(), test_pipeline.get_layout());

        // draw (after recording)
        new_mesh.draw(current);
        // sphere_mesh.draw(current);

        ImGui::Begin("Viewport");
        ImGui::Button("Texture Image 0");

        DrawVec3UI("Position", position);
        DrawVec3UI("Rotation", rotation);
        DrawVec3UI("Cam Pos", test_camera.Position);
        DrawVec3UI("Color", color);
        // DrawVec3UI("")

        // ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        // ImGui::Image(test_descriptor_sets.get(frame), ImVec2{100, 100});

        // ImGui::Image()
        ImGui::End();

        test_imgui.end(current);
        main_window_swapchain.end(current);

        //! @note This submits the command buffer and also presents the command buffer as well
        main_window_swapchain.submit(current);


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

    //! @note This would probably be something specified in something like vk_context::submit_resource_free([](){ m_command_buffers.destroy(); })
    //! @note Whenever something needs to be submitted and destroyed when the application shuts down
    test_imgui.destroy();

    my_texture.destroy();
    for (size_t i = 0; i < test_uniforms.size(); i++) {
        test_uniforms[i].destroy();
    }

    // test_descriptor_sets.destroy();
    // new_mesh.destroy();
    // test_pipeline.destroy();
    // test_shader.destroy();
    // main_window_swapchain.destroy();
    // main_driver.destroy();
    initiating_vulkan.cleanup();
}
