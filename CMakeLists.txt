cmake_minimum_required(VERSION 3.27)
project(Starter CXX)

set(INCLUDE_DIR vulkan-cpp)
set(SRC_DIR src/vulkan-cpp)

set(
    all_headers
    ${INCLUDE_DIR}/vk_window.hpp
    ${INCLUDE_DIR}/vk_context.hpp
    ${INCLUDE_DIR}/vk_physical_driver.hpp
    ${INCLUDE_DIR}/vk_driver.hpp
    ${INCLUDE_DIR}/vk_swapchain.hpp
    ${INCLUDE_DIR}/vk_queue.hpp

    ${INCLUDE_DIR}/vk_shader.hpp
    ${INCLUDE_DIR}/vk_pipeline.hpp

    ${INCLUDE_DIR}/vk_descriptor_set.hpp
    ${INCLUDE_DIR}/vk_uniform_buffer.hpp
    ${INCLUDE_DIR}/vk_texture.hpp
    ${INCLUDE_DIR}/vk_command_buffer.hpp

    ${INCLUDE_DIR}/vk_vertex_buffer.hpp
    ${INCLUDE_DIR}/vk_index_buffer.hpp
    ${INCLUDE_DIR}/vk_renderpass.hpp
    ${INCLUDE_DIR}/helper_functions.hpp

    ${INCLUDE_DIR}/vk_imgui.hpp

    ${INCLUDE_DIR}/logger.hpp
    renderer/mesh.hpp
    ${INCLUDE_DIR}/perspective_camera.hpp
)

set(
    all_src
    application.cpp
    ${SRC_DIR}/vk_window.cpp
    ${SRC_DIR}/vk_context.cpp
    ${SRC_DIR}/vk_physical_driver.cpp
    ${SRC_DIR}/vk_driver.cpp
    ${SRC_DIR}/vk_swapchain.cpp
    ${SRC_DIR}/vk_queue.cpp
    
    ${SRC_DIR}/vk_shader.cpp
    ${SRC_DIR}/vk_pipeline.cpp
    ${SRC_DIR}/vk_descriptor_set.cpp
    ${SRC_DIR}/vk_uniform_buffer.cpp
    ${SRC_DIR}/vk_command_buffer.cpp

    ${SRC_DIR}/vk_texture.cpp

    ${SRC_DIR}/vk_vertex_buffer.cpp
    ${SRC_DIR}/vk_index_buffer.cpp

    ${SRC_DIR}/vk_imgui.cpp

    src/renderer/mesh.cpp
    # ${SRC_DIR}/perspective_camera.cpp
    
    ${SRC_DIR}/vk_renderpass.cpp
    ${SRC_DIR}/helper_functions.cpp
    ${SRC_DIR}/logger.cpp
)


build_application(
    SOURCES
    ${all_headers}
    ${all_src}

    PACKAGES
    glfw3
    fmt
    spdlog
    glm
    imguidocking
    VulkanHeaders
    stb
    Vulkan
    flecs
    tinyobjloader
    imguidocking

    LINK_PACKAGES
    glfw
    fmt::fmt
    spdlog::spdlog
    glm::glm
    imguidocking::imguidocking
    vulkan-headers::vulkan-headers
    stb::stb
    Vulkan::Vulkan
    flecs::flecs_static
    tinyobjloader::tinyobjloader
    imguidocking::imguidocking
)

generate_compile_commands()


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})