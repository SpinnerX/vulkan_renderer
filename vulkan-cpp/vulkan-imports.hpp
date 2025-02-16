#pragma once

#define GLFW_INCLUDE_VULKAN
#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
// #include "vulkan_win32.h"
#include <vulkan/vulkan_win32.h>
#endif
// #ifdef VK_USE_PLATFORM_MACOS_MVK
#ifdef __APPLE__
// #include "vulkan_macos.h"
#include <vulkan/vulkan_macos.h>
#ifdef VK_USE_PLATFORM_METAL_EXT
// #include "vulkan_metal.h"
#include <vulkan/vulkan_metal.h>
#endif

#endif
#include <vulkan/vulkan.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#endif