#include <vulkan/vulkan_core.h>
#include <fmt/core.h>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <vulkan-cpp/vk_driver.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/vk_shader.hpp>
#include <vulkan-cpp/vk_pipeline.hpp>

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

    int width = 800;
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
    vk::vk_physical_driver main_physical_device = vk::vk_physical_driver(initiating_vulkan);
    vk::vk_driver main_driver = vk::vk_driver(main_physical_device);

    //! @note 4.) Initializing Swapchain
    vk::vk_swapchain main_window_swapchain = vk::vk_swapchain(main_physical_device, main_driver, main_window);

    vk::vk_shader test_shader = vk::vk_shader("shaders/vert.spv", "shaders/frag.spv");
    // vk::vk_pipeline test_pipeline = vk::vk_pipeline(main_window, main_window_swapchain.get_renderpass(), {test_shader.get_vertex_module(), test_shader.get_fragment_module()});

    // recording clear colors for all swapchain command buffers
    main_window_swapchain.record([&main_window_swapchain](const VkCommandBuffer& p_command_buffer){
        // test_pipeline.bind(p_command_buffer);

        // VkViewport viewport = {
        //     .x = 0.0f,
        //     .y = 0.0f,
        //     .width = static_cast<float>(main_window_swapchain.get_extent().width),
        //     .height = static_cast<float>(main_window_swapchain.get_extent().height),
        //     .minDepth = 0.0f,
        //     .maxDepth = 1.0f,
        // };
        // vkCmdSetViewport(p_command_buffer, 0, 1, &viewport);

        // VkRect2D scissor = {
        //     .offset = {0, 0},
        //     .extent = main_window_swapchain.get_extent(),
        // };

        // vkCmdSetScissor(p_command_buffer, 0, 1, &scissor);

        // uint32_t vertex_count = 3;
        // uint32_t instance_count = 1;
        // uint32_t first_vertex = 0;
        // uint32_t first_instance = 0;
        // vkCmdDraw(p_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
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
    // test_pipeline.destroy();
    test_shader.destroy();
    main_window_swapchain.destroy();
    main_driver.destroy();

}