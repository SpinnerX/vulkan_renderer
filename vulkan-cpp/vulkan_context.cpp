#include "vulkan_context.hpp"
#include <fmt/core.h>
#include <vector>
#include <string>
#include "vulkan-imports.hpp"
#include "helper_functions.hpp"
#include <vulkan/vulkan_core.h>

namespace vk{
    context* context::s_Instance = nullptr;

    static std::vector<const char*> initialize_instance_extensions(){
        std::vector<const char*> extensionNames;

        extensionNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

        // An additional surface extension needs to be loaded. This extension is platform-specific so needs to be selected based on the
        // platform the example is going to be deployed to.
        // Preprocessor directives are used here to select the correct platform.
    #ifdef VK_USE_PLATFORM_WIN32_KHR
        extensionNames.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef VK_USE_PLATFORM_XLIB_KHR
        extensionNames.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef VK_USE_PLATFORM_XCB_KHR
        extensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef VK_USE_PLATFORM_ANDROID_KHR
        extensionNames.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef VK_USE_PLATFORM_WAYLAND_KHR
        extensionNames.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef VK_USE_PLATFORM_MACOS_MVK
        extensionNames.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
    #endif
    #ifdef USE_PLATFORM_NULLWS
        extensionNames.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    #endif

        return extensionNames;
    }

    static std::vector<const char*> initialize_validation_layers(){
        uint32_t layer_count;

        //! @note Validation layers that will be returned.
        std::vector<const char*> layer_names;
        layer_names.push_back("VK_LAYER_KHRONOS_validation");

        //! @note Enumerating the layer size
        vk_check(vkEnumerateInstanceLayerProperties(&layer_count, nullptr), "vkEnumerateInstanceLayerProperties (1)", __FILE__, __LINE__, __FUNCTION__);
        std::vector<VkLayerProperties> layer_properties;
        layer_properties.resize(layer_count);

        vk_check(vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data()), "vkEnumerateInstanceLayerProperties (1)", __FILE__, __LINE__, __FUNCTION__);

        for(const auto& layer : layer_properties){
            for(uint32_t j = 0; j < layer_count; j++){
                // if(layer_properties[j].layerName == layer.layerName)
                if(strcmp(layer_properties[j].layerName, layer.layerName)){
                    layer_names.emplace_back(layer_properties[j].layerName);
                }
            }
        }
        return layer_names;
    }

    context::context(){
        fmt::println("Vulkan Context Begin Initialization!");

        s_Instance = this;
        const std::vector<std::string> InstanceLayers = {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_standard_validation",
            // PerfDoc is a Vulkan layer which attempts to identify API usage that may be discouraged, primarily by validating applications
            // against the rules set out in the Mali Application Developer Best Practices document.
            "VK_LAYER_ARM_mali_perf_doc"
        };

        //! @note Setting up vulkan's API for application
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .applicationVersion = 1,
            .pEngineName = "Engine3D",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_3,
        };

        VkInstanceCreateInfo instance_create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info
        };

        VkInstanceCreateInfo instance_ci = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info
        };

        std::vector<const char*> validation_layers = initialize_validation_layers();
        std::vector<const char*> instance_extensions = initialize_instance_extensions();

        instance_ci.enabledExtensionCount = static_cast<uint32_t>(validation_layers.size());
        instance_ci.ppEnabledExtensionNames = validation_layers.data();

        instance_ci.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
        instance_ci.ppEnabledExtensionNames = instance_extensions.data();

        vk_check(vkCreateInstance(&instance_ci, nullptr, &m_instance), "vkCreateInstance", __FILE__, __LINE__, __FUNCTION__);

        fmt::println("Vulkan Instance created!");
    }
};