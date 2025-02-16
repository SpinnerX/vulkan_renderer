cmake_minimum_required(VERSION 3.27)
project(Starter CXX)


build_application(
    SOURCES
    Application.cpp

    vulkan-cpp/vulkan_context.cpp
    vulkan-cpp/vulkan_physical_driver.cpp
    vulkan-cpp/vulkan_driver.cpp

    vulkan-cpp/vulkan_swapchain.cpp
    vulkan-cpp/helper_functions.cpp

    PACKAGES
    glfw3
    fmt
    glm
    imguidocking
    VulkanHeaders
    stb
    Vulkan

    LINK_PACKAGES
    glfw
    fmt::fmt
    glm::glm
    imguidocking::imguidocking
    vulkan-headers::vulkan-headers
    stb::stb
    Vulkan::Vulkan
)
