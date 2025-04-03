#include <vulkan-cpp/vk_imgui.hpp>
#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/vk_swapchain.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <vulkan-cpp/vk_window.hpp>
#include <vulkan-cpp/vk_context.hpp>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {
    static void imgui_color_layout_customization() {
        auto& colors = ImGui::GetStyle().Colors; // @note Colors is ImVec4

        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.15f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] =
          ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Titles
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.15f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] =
          ImVec4{ 0.1f, 0.150f, 0.951f, 1.0f };
    }

    vk_imgui::vk_imgui() {
        m_driver = vk_driver::driver_context();

        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
        };

        VkDescriptorPoolCreateInfo desc_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000 * IM_ARRAYSIZE(pool_sizes),
            // .poolSizeCount = (uint32_t)std::size(pool_sizes),
            .poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes),
            .pPoolSizes = pool_sizes
        };

        // VkDescriptorPool imgui_pool;
        vk::vk_check(
          vkCreateDescriptorPool(
            m_driver, &desc_pool_create_info, nullptr, &m_imgui_desc_pool),
          "vkCreateDescriptorPool",
          __FUNCTION__);
        console_log_info("After creating descriptor sets for IMGUI");
    }

    void vk_imgui::initialize(const VkInstance& p_instance,
                              const VkPhysicalDevice& p_physical,
                              const vk_swapchain& p_swapchain) {
        m_current_swapchain = p_swapchain;
        console_log_info("Imgui Debug Track #0");
        //! @note Setting up imgui stuff.
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |=
          ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable
        // Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |=
          ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform
                                            // Windows
        console_log_info("Imgui Debug Track #1");

        // Setting custom dark themed imgui layout
        imgui_color_layout_customization();

        console_log_info("Imgui Debug Track #2");

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        console_log_info("Imgui Debug Track #3");

        // VkRenderPass rp = p_renderpass;
        // console_log_trace("Render Pass Memory Access = {}", (void*)rp);
        // if(rp == nullptr) {
        //     console_log_fatal("VkRenderPass from swapchain == nullptr!!!");
        // }
        console_log_info("Imgui Debug Track #4");

        // this initializes imgui for Vulkan
        ImGui_ImplGlfw_InitForVulkan(vk_window::native_window(), true);
        console_log_info("Imgui Debug Track #4.1");
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = p_instance;
        console_log_info("Imgui Debug Track #4.2");
        init_info.PhysicalDevice = p_physical;
        console_log_info("Imgui Debug Track #4.3");
        init_info.Device = m_driver;
        console_log_info("Imgui Debug Track #4.4");
        init_info.Queue = m_driver.get_graphics_queue();
        console_log_info("Imgui Debug Track #4.5");
        init_info.RenderPass = p_swapchain.get_renderpass();
        console_log_info("Imgui Debug Track #4.6");
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = m_imgui_desc_pool;
        console_log_info("Imgui Debug Track #4.7");
        init_info.MinImageCount = 2;
        init_info.ImageCount = p_swapchain.image_size();
        console_log_info("Imgui Debug Track #4.8");
        init_info.UseDynamicRendering = false;
        console_log_info("Imgui Debug Track #5");

        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        ImGui_ImplVulkan_Init(&init_info);
        console_log_warn("After ImGui_ImplGlfw_InitForVulkan called!");

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void vk_imgui::begin() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void vk_imgui::end() {
        ImGui::Render();

        // auto current_cmd_buffer = get_current_command_buffer();
        // VkCommandBuffer current = vk_swapchain::current_active_buffer();
        VkCommandBuffer current =
          m_current_swapchain.current_active_comand_buffer();

        int width, height;
        glfwGetFramebufferSize(vk_window::native_window(),
                               &width,
                               &height); // Or use glfwget_windowSize
        // updateViewport(
        //     current,
        //   width,
        //   height); // Pass window width and height for initial viewport setup

        //! @note This works, dont modify.
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, current);

        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void vk_imgui::destroy() {
        vkDestroyDescriptorPool(m_driver, m_imgui_desc_pool, nullptr);
    }
};