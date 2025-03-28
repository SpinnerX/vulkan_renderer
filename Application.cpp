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

    vk::vk_shader test_shader = vk::vk_shader("shaders/vert.spv", "shaders/frag.spv");

    // adding descriptor sets

    std::vector<vk::vertex> vertices = {
        {
            .Position = {-1.f, -1.f, 0.f},
            .TexCoords = {0.0f, 0.f}
        },
        {
            .Position = {1.f, -1.f, 0.f},
            .TexCoords = {0.0f, 1.f}
        },
        {
            .Position = {0.f, -1.f, 0.f},
            .TexCoords = {1.0f, 1.f}
        }
    };
    vk::vk_vertex_buffer test_vertex_buffer = vk::vk_vertex_buffer(vertices);

    vk::vk_descriptor_set descriptor_sets = vk::vk_descriptor_set(main_window_swapchain.image_size());

    vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window, main_window_swapchain.get_renderpass(), test_shader.get_vertex_module(), test_shader.get_fragment_module(), descriptor_sets, test_vertex_buffer);

    descriptor_sets.update_descriptor_sets(test_vertex_buffer);
    // recording clear colors for all swapchain command buffers
    main_window_swapchain.record([&main_window_swapchain, &test_pipeline, &test_vertex_buffer, &descriptor_sets](const VkCommandBuffer& p_command_buffer){
        test_pipeline.bind(p_command_buffer);

        if(descriptor_sets.descriptors_count() > 0) {
            VkDescriptorSet desc_set = descriptor_sets.get(main_window_swapchain.current_frame());
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

        uint32_t vertex_count = 3;
        uint32_t instance_count = 1;
        uint32_t first_vertex = 0;
        uint32_t first_instance = 0;
        vkCmdDraw(p_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
        test_vertex_buffer.bind(p_command_buffer);
    });

    while(main_window.is_active()){

        // acquire next image ( then record)


        // draw (after recording)

        // presenting frame (after drawing that frame)
        main_window_swapchain.render_scene();
        
        glfwPollEvents();
    }

    // Lets make sure we destroy these objects in the order they're created

    // tell device to wait before destroying everything
    // doing this to ensure that we destroy them after everrythings done executing
    vkDeviceWaitIdle(main_driver);
    descriptor_sets.destroy();
    test_vertex_buffer.destroy();
    test_pipeline.destroy();
    test_shader.destroy();
    main_window_swapchain.destroy();
    main_driver.destroy();

}