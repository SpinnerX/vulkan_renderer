#pragma once
#include <vulkan-cpp/vk_driver.hpp>
#include <array>

namespace vk {
    struct swapchain_properties {
        static constexpr uint32_t MaxFramesInFlight = 2;
    };
    class vk_swapchain {
    public:
        vk_swapchain() = default;
        vk_swapchain(const vk_physical_driver& p_physical, const vk_driver& p_driver, const VkSurfaceKHR& p_surface);
        ~vk_swapchain();

        void resize(int p_width, int p_height);


        // bool is_reset() const { return m_swapchain_rebuild; }

    private:
        void on_create();


    private:
        void select_swapchain_surface_formats();

        void select_swapchain_surface_capabilites();

        void select_swapchain_current_extent();

        void select_swapchain_present_modes();

        VkExtent2D get_valid_swapchain_extent();

        //! @note Doing image's stuff for swapchain


    private:
        // bool m_swapchain_rebuild = false;
        struct image {
            VkImage Image=nullptr;
            VkImageView ImageView=nullptr;
        };

        vk_physical_driver m_physical;
        vk_driver m_driver;

        VkExtent2D m_swapchain_size;

        // surface properties
        VkSurfaceCapabilitiesKHR m_surface_capabilities;
        VkSurfaceFormatKHR  m_surface_format;
        VkSurfaceKHR m_current_surface;

        //! @note Properties for presenting to the display itself from this swapchain
        uint32_t m_present_index = -1;
        VkPresentModeKHR m_presentation_mode;
        VkQueue m_presentation_queue=nullptr;

        //! @note The actual swapchain handler
        VkSwapchainKHR m_swapchain_handler=nullptr;

        //! @note Image's handling variables here
        uint32_t m_image_count = -1;

        std::array<image, swapchain_properties::MaxFramesInFlight> m_swapchain_images;
    };
};