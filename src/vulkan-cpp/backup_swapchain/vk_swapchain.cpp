#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>
#include <vulkan-cpp/logger.hpp>
#include <array>

namespace vk {
    VkPresentModeKHR select_compatible_present_mode(
        const VkPresentModeKHR& p_request,
        const std::vector<VkPresentModeKHR>& p_modes) {
          for (const auto& mode : p_modes) {
              if (mode == p_request) {
                  return mode;
              }
          }
          return VK_PRESENT_MODE_FIFO_KHR;
      }
    vk_swapchain::vk_swapchain(const vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface) :m_physical(p_physical), m_driver(p_driver), m_current_surface(p_surface){

        //! @note First get surface format
        select_swapchain_surface_formats();

        //! @note Second get swapchain extent capabilities
        select_swapchain_surface_capabilites();

        //! @note Getting present mode = 0
        //! @note TO get presentation mode surface capabilities are required
        select_swapchain_present_modes();

        m_present_index = m_physical.get_presentation_index(p_surface);
        console_log_trace("present index = {}", (int)m_present_index);

        // vkGetDeviceQueue(m_driver, m_present_index, 0, &m_presentation_queue);

        m_swapchain_size = get_valid_swapchain_extent();

        on_create();
    }

    vk_swapchain::~vk_swapchain(){
        console_log_trace("~vk_swapchain called!!!");

        if(m_swapchain_handler != nullptr){
            vkDestroySwapchainKHR(m_driver, m_swapchain_handler, nullptr);
        }
    }

    void vk_swapchain::on_create() {

        console_log_info("vk_swapchain::on_create begin initialization!!!");
        //! @note Getting surface image count
        // Extract min num of images supported on our current surface.
        uint32_t swapchain_images_count;
        std::vector<VkImage> images;

        // We default set to the minimum image count
        m_image_count = m_surface_capabilities.minImageCount + 1;

        uint32_t surface_img_count = std::max<uint32_t>(3, m_surface_capabilities.minImageCount);

        VkSwapchainCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_current_surface,

            .minImageCount = m_image_count,
            .imageFormat = m_surface_format.format,
            .imageColorSpace = m_surface_format.colorSpace,
            .imageExtent = m_swapchain_size,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };


        if(m_physical.get_queue_indices().Graphics != m_present_index){
            std::array<uint32_t, 2> queue_indices = {m_physical.get_queue_indices().Graphics, m_present_index};
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_indices.size());
            create_info.pQueueFamilyIndices = queue_indices.data();
        }
        else{
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        create_info.preTransform = m_surface_capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = m_presentation_mode;
        create_info.clipped = true;
        create_info.oldSwapchain = VK_NULL_HANDLE; // used when resizing


        console_log_trace("Present Mode = {}", vk_present_mode_to_string(m_presentation_mode));
        console_log_trace("Present Mode = {}", vk_present_mode_to_string(create_info.presentMode));

       vk_check(
        vkCreateSwapchainKHR(m_driver, &create_info, nullptr, &m_swapchain_handler),
        "vkCreateSwapchainKHR",
        __FUNCTION__);

        console_log_info("vk_swapchain::on_create end initialization!!!");
    }
    


    void vk_swapchain::select_swapchain_surface_formats(){
        uint32_t format_count = 0;
        std::vector<VkSurfaceFormatKHR> formats;
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_current_surface, &format_count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FUNCTION__);
    
        formats.resize(format_count);

        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical, m_current_surface, &format_count, formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR", __FUNCTION__);

        for(const auto& format : formats){
            if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                m_surface_format = format;
            }
        }

        if(formats.size() == 0){
            m_surface_format = formats[0];
        }
    }

    void vk_swapchain::select_swapchain_surface_capabilites() {
        // VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical, m_current_surface, &m_surface_capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",__FUNCTION__);

        //! @note Extracting number of presentation modes.
        uint32_t preset_modes_count;
        std::vector<VkPresentModeKHR> presentation_modes;
        vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
                   m_physical, m_current_surface, &preset_modes_count, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR",__FUNCTION__);
        presentation_modes.resize(preset_modes_count);
        vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical, m_current_surface, &preset_modes_count, presentation_modes.data()), "vkGetPhysicalDeviceSurfacePresentModesKHR",__FUNCTION__);

        m_presentation_mode = select_compatible_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR, presentation_modes);
    }

    void vk_swapchain::select_swapchain_present_modes() {
        //! @note Extracting number of presentation modes.
        uint32_t preset_modes_count;
        std::vector<VkPresentModeKHR> presentation_modes;
        vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
                   m_physical, m_current_surface, &preset_modes_count, nullptr),
                 "vkGetPhysicalDeviceSurfacePresentModesKHR",
                 __FUNCTION__);
        presentation_modes.resize(preset_modes_count);
        vk_check(
          vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical,
                                                    m_current_surface,
                                                    &preset_modes_count,
                                                    presentation_modes.data()),
          "vkGetPhysicalDeviceSurfacePresentModesKHR",
          __FUNCTION__);

        m_presentation_mode = select_compatible_present_mode(
          VK_PRESENT_MODE_IMMEDIATE_KHR, presentation_modes);

    }

    void vk_swapchain::select_swapchain_current_extent(){
        
        m_swapchain_size = get_valid_swapchain_extent();
    }

    VkExtent2D vk_swapchain::get_valid_swapchain_extent() {

        //! @note Width/Height of our current swapchain

        //! @note All this checks is the width/height are not set, then whatever
        //! the swapchain is set to will set the extent by default
        if(m_surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return m_surface_capabilities.currentExtent;
        }

        return m_surface_capabilities.currentExtent;
    }

    void vk_swapchain::resize(int p_width, int p_height) {
        m_swapchain_size.width = p_width;
        m_swapchain_size.height = p_height;
    }
};