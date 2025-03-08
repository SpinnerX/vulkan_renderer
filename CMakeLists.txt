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
    ${INCLUDE_DIR}/vk_renderpass.hpp
    ${INCLUDE_DIR}/helper_functions.hpp
    ${INCLUDE_DIR}/logger.hpp
)

set(
    all_src
    application.cpp
    ${SRC_DIR}/vk_window.cpp
    ${SRC_DIR}/vk_context.cpp
    ${SRC_DIR}/vk_physical_driver.cpp
    ${SRC_DIR}/vk_driver.cpp
    ${SRC_DIR}/vk_swapchain.cpp
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

    LINK_PACKAGES
    glfw
    fmt::fmt
    spdlog::spdlog
    glm::glm
    imguidocking::imguidocking
    vulkan-headers::vulkan-headers
    stb::stb
    Vulkan::Vulkan
)

generate_compile_commands()


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})