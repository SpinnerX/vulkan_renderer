#include <vulkan-cpp/vk_context.hpp>
#include <iostream>
#include <vulkan-cpp/helper_functions.hpp>
#include <GLFW/glfw3.h>
#include <vulkan-cpp/logger.hpp>


namespace vk {
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_KHRONOS_synchronization2"
    };
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer:\t\t" << pCallbackData->pMessage << std::endl;
        return false;
    }

    static std::vector<const char*> get_required_extensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }
    
    vk_context* vk_context::s_instance = nullptr;
    vk_context::vk_context(const std::string& p_tag) {
        console_log_info("vk_context::vk_context begin initialization!!!");
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .applicationVersion = 1,
            .pEngineName = "renderer",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_3,
        };
    
        VkInstanceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info
        };

        //! @note Setting up the required extensions for vulkan
        std::vector<const char*> extensions = get_required_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        
        // printing out available validation layers
        uint32_t layer_count;
        std::vector<VkLayerProperties> available_validation_layers;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        available_validation_layers.resize(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_validation_layers.data());

        console_log_trace("================================================");
        console_log_trace("\tValidation Layers Available");
        console_log_trace("================================================");
        for(VkLayerProperties properties : available_validation_layers) {
            console_log_trace("Validation Layer:\t\t{}", properties.layerName);
            console_log_trace("Description\t\t{}", properties.description);
            console_log_trace("Version\t\t\t{}", (int)properties.specVersion);
        }
        console_log_trace("================================================");




        // by default we enable validation layers used for debugging!
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
        };
    
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    
        vk::vk_check(vkCreateInstance(&create_info, nullptr, &m_instance), "vkCreateInstance", __FUNCTION__);

        s_instance = this;
        console_log_info("vk_context::vk_context end initialization!!!\n\n");
    }

    vk_context::~vk_context(){
        // vkDestroyInstance(m_instance, nullptr);
    }

    void vk_context::cleanup() {
        vkDestroyInstance(m_instance, nullptr);
    }

};