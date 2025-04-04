#include <vulkan-cpp/helper_functions.hpp>
#include <vulkan-cpp/logger.hpp>
#include <vulkan-cpp/vk_driver.hpp>

namespace vk {

    VkCommandBufferBeginInfo commend_buffer_begin_info(
      const VkCommandBufferUsageFlags& p_usage) {
        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = p_usage,
            .pInheritanceInfo = nullptr
        };

        return begin_info;
    }

    VkCommandPool create_single_command_pool() {
        VkDevice driver = vk_driver::driver_context();
        vk_physical_driver physical = vk_physical_driver::physical_driver();
        uint32_t graphics_queue_index = physical.get_queue_indices().Graphics;
        VkCommandPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphics_queue_index
        };

        VkCommandPool command_pool = nullptr;
        vk_check(vkCreateCommandPool(driver, &pool_ci, nullptr, &command_pool),
                 "vkCreateCommandPool",
                 __FUNCTION__);

        return command_pool;
    }

    VkCommandBuffer create_single_command_buffer(
      const VkCommandPool& p_command_pool) {
        VkDevice driver = vk_driver::driver_context();

        VkCommandBufferAllocateInfo command_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = p_command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer command_buffer = nullptr;
        vk_check(vkAllocateCommandBuffers(
                   driver, &command_buffer_alloc_info, &command_buffer),
                 "vkAllocateCommandBuffers",
                 __FUNCTION__);

        return command_buffer;
    }

    void begin_command_buffer(const VkCommandBuffer& p_command_buffer,
                              VkCommandBufferUsageFlags p_usage_flags) {
        VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = p_usage_flags,
            .pInheritanceInfo = nullptr
        };

        vk_check(
          vkBeginCommandBuffer(p_command_buffer, &command_buffer_begin_info),
          "vkBeginCommandBuffer",
          __FUNCTION__);
    }

    void end_command_buffer(const VkCommandBuffer& p_command_buffer) {
        vkEndCommandBuffer(p_command_buffer);
    }

    buffer_properties create_buffer(uint32_t p_device_size,
                                    VkBufferUsageFlags p_usage,
                                    VkMemoryPropertyFlags p_property_flags) {
        vk_driver driver = vk_driver::driver_context();

        if (driver != nullptr) {
            console_log_warn("driver is still valid!!!");
        }

        buffer_properties new_buffer{};
        new_buffer.AllocateDeviceSize = p_device_size;

        VkBufferCreateInfo buffer_ci = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = new_buffer.AllocateDeviceSize, // size in bytes
            .usage = p_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        // 1. creating our buffer
        vk_check(vkCreateBuffer(
                   driver, &buffer_ci, nullptr, &new_buffer.BufferHandler),
                 "vkCreateBuffer",
                 __FUNCTION__);

        // 2.  getting buffer memory requirements
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(
          driver, new_buffer.BufferHandler, &memory_requirements);

        // 3. get memory type index
        uint32_t memory_type_index = driver.select_memory_type(
          memory_requirements.memoryTypeBits, p_property_flags);

        // 4. allocate memory
        /**
         * Memory Type Index
         * - Physical device enumerate all the physical hardware on your machine
         *
         */
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(
          vkAllocateMemory(
            driver, &memory_alloc_info, nullptr, &new_buffer.DeviceMemory),
          "vkAllocateMemory",
          __FUNCTION__);

        // 5. bind memory
        vk_check(
          vkBindBufferMemory(
            driver, new_buffer.BufferHandler, new_buffer.DeviceMemory, 0),
          "vkBindBufferMemory",
          __FUNCTION__);

        return new_buffer;
    }

    void write(const buffer_properties& p_buffer,
               const void* p_data,
               size_t p_size_in_bytes) {
        VkDevice driver = vk_driver::driver_context();
        void* mapped = nullptr;
        vk_check(
          vkMapMemory(
            driver, p_buffer.DeviceMemory, 0, p_size_in_bytes, 0, &mapped),
          "vkMapMemory",
          __FUNCTION__);
        memcpy(mapped, p_data, p_size_in_bytes);
        vkUnmapMemory(driver, p_buffer.DeviceMemory);
    }

    void write(const buffer_properties& p_buffer,
               const std::span<uint32_t>& p_in_buffer) {
        VkDeviceSize buffer_size = p_in_buffer.size_bytes();
        VkDevice driver = vk_driver::driver_context();
        void* mapped = nullptr;
        vk_check(vkMapMemory(
                   driver, p_buffer.DeviceMemory, 0, buffer_size, 0, &mapped),
                 "vkMapMemory",
                 __FUNCTION__);
        memcpy(mapped, p_in_buffer.data(), buffer_size);
        vkUnmapMemory(driver, p_buffer.DeviceMemory);
    }


    void copy(const buffer_properties& p_src, const buffer_properties& p_dst, uint32_t p_size_of_bytes) {
        VkDevice driver = vk_driver::driver_context();
        VkQueue graphics_queue =vk_driver::driver_context().get_graphics_queue();
        VkCommandPool command_pool = create_single_command_pool();
        VkCommandBuffer copy_cmd_buffer = create_single_command_buffer(command_pool);

        begin_command_buffer(copy_cmd_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkBufferCopy copy_region{};
        copy_region.size = (VkDeviceSize)p_size_of_bytes;
        vkCmdCopyBuffer(copy_cmd_buffer, p_src.BufferHandler, p_dst.BufferHandler, 1, &copy_region);
        end_command_buffer(copy_cmd_buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &copy_cmd_buffer;
        vkQueueSubmit(graphics_queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(graphics_queue);

        vkFreeCommandBuffers(driver, command_pool, 1, &copy_cmd_buffer);
        vkDestroyCommandPool(driver, command_pool, nullptr);
    }

    void write(const buffer_properties& p_buffer,
               const std::span<float>& p_in_buffer) {
        VkDeviceSize buffer_size =
          p_in_buffer
            .size_bytes(); // does equivalent to doing sizeof(p_in_buffer[0]) *
                           // p_in_buffer.size();
        VkDevice driver = vk_driver::driver_context();
        void* mapped = nullptr;
        vk_check(vkMapMemory(
                   driver, p_buffer.DeviceMemory, 0, buffer_size, 0, &mapped),
                 "vkMapMemory",
                 __FUNCTION__);
        memcpy(mapped, p_in_buffer.data(), buffer_size);
        vkUnmapMemory(driver, p_buffer.DeviceMemory);
    }

    void write(const buffer_properties& p_buffer,
               const std::span<vertex>& p_in_buffer) {
        VkDeviceSize buffer_size =
          p_in_buffer
            .size_bytes(); // does equivalent to doing sizeof(p_in_buffer[0]) *
                           // p_in_buffer.size();
        VkDevice driver = vk_driver::driver_context();
        void* mapped = nullptr;
        vk_check(vkMapMemory(
                   driver, p_buffer.DeviceMemory, 0, buffer_size, 0, &mapped),
                 "vkMapMemory",
                 __FUNCTION__);
        memcpy(mapped, p_in_buffer.data(), buffer_size);
        vkUnmapMemory(driver, p_buffer.DeviceMemory);
    }

    void vk_check(const VkResult& result,
                  const char* p_tag,
                  const char* p_function_name,
                  const char* p_filepath,
                  uint32_t p_line) {
        if (result != VK_SUCCESS) {
            console_log_error_tagged("vulkan",
                                     "VkResult failed taking in {0} file: {1} "
                                     "--- Line: {2} --- In Function: {3}",
                                     p_tag,
                                     p_filepath,
                                     p_line,
                                     p_function_name);
            console_log_error_tagged(
              "vulkan", "VkResult returned: {}", (int)result);
        }
    }

    void vk_check_format(VkFormat p_format,
                         const char* p_function_name,
                         const char* p_filepath,
                         uint32_t p_line) {
        if (p_format == VK_FORMAT_UNDEFINED) {
            console_log_error_tagged(
              "vulkan",
              "VkFormat failed in file: {0} --- Line: {1} --- In Function: {2}",
              p_filepath,
              p_line,
              p_function_name);
            console_log_error_tagged(
              "vulkan", "VkFormat was assigned to VK_FORMAT_UNDEFINED");
        }
    }

    std::string vk_queue_flags_to_string(VkQueueFlagBits p_flags) {
        switch (p_flags) {
            case VK_QUEUE_GRAPHICS_BIT:
                return "VK_QUEUE_GRAPHICS_BIT";
            case VK_QUEUE_COMPUTE_BIT:
                return "VK_QUEUE_COMPUTE_BIT";
            case VK_QUEUE_TRANSFER_BIT:
                return "VK_QUEUE_TRANSFER_BIT";
            case VK_QUEUE_SPARSE_BINDING_BIT:
                return "VK_QUEUE_SPARSE_BINDING_BIT";
            case VK_QUEUE_PROTECTED_BIT:
                return "VK_QUEUE_PROTECTED_BIT";
            case VK_QUEUE_VIDEO_DECODE_BIT_KHR:
                return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
            case VK_QUEUE_VIDEO_ENCODE_BIT_KHR:
                return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
            case VK_QUEUE_OPTICAL_FLOW_BIT_NV:
                return "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
            case VK_QUEUE_FLAG_BITS_MAX_ENUM:
                return "VK_QUEUE_FLAG_BITS_MAX_ENUM";
        }

        return "VkQueueFlag not Selected";
    }

    std::string vk_present_mode_to_string(VkPresentModeKHR p_present_mode) {
        switch (p_present_mode) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return "VK_PRESENT_MODE_IMMEDIATE_KHR";
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return "VK_PRESENT_MODE_MAILBOX_KHR";
            case VK_PRESENT_MODE_FIFO_KHR:
                return "VK_PRESENT_MODE_FIFO_KHR";
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
            case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
                return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
            case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
                return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
            case VK_PRESENT_MODE_MAX_ENUM_KHR:
                return "VK_PRESENT_MODE_MAX_ENUM_KHR";
            default:
                return "Present Mode Unspecified!";
        }
    }

    std::string vk_format_to_string(VkFormat p_format) {
        switch (p_format) {
            case VK_FORMAT_UNDEFINED:
                return "VK_FORMAT_UNDEFINED";
            case VK_FORMAT_R4G4_UNORM_PACK8:
                return "VK_FORMAT_R4G4_UNORM_PACK8";
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
                return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
                return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
                return "VK_FORMAT_R5G6B5_UNORM_PACK16";
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
                return "VK_FORMAT_B5G6R5_UNORM_PACK16";
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
                return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
                return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
                return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
            case VK_FORMAT_R8_UNORM:
                return "VK_FORMAT_R8_UNORM";
            case VK_FORMAT_R8_SNORM:
                return "VK_FORMAT_R8_SNORM";
            case VK_FORMAT_R8_USCALED:
                return "VK_FORMAT_R8_USCALED";
            case VK_FORMAT_R8_SSCALED:
                return "VK_FORMAT_R8_SSCALED";
            case VK_FORMAT_R8_UINT:
                return "VK_FORMAT_R8_UINT";
            case VK_FORMAT_R8_SINT:
                return "VK_FORMAT_R8_SINT";
            case VK_FORMAT_R8_SRGB:
                return "VK_FORMAT_R8_SRGB";
            case VK_FORMAT_R8G8_UNORM:
                return "VK_FORMAT_R8G8_UNORM";
            case VK_FORMAT_R8G8_SNORM:
                return "VK_FORMAT_R8G8_SNORM";
            case VK_FORMAT_R8G8_USCALED:
                return "VK_FORMAT_R8G8_USCALED";
            case VK_FORMAT_R8G8_SSCALED:
                return "VK_FORMAT_R8G8_SSCALED";
            case VK_FORMAT_R8G8_UINT:
                return "VK_FORMAT_R8G8_UINT";
            case VK_FORMAT_R8G8_SINT:
                return "VK_FORMAT_R8G8_SINT";
            case VK_FORMAT_R8G8_SRGB:
                return "VK_FORMAT_R8G8_SRGB";
            case VK_FORMAT_R8G8B8_UNORM:
                return "VK_FORMAT_R8G8B8_UNORM";
            case VK_FORMAT_R8G8B8_SNORM:
                return "VK_FORMAT_R8G8B8_SNORM";
            case VK_FORMAT_R8G8B8_USCALED:
                return "VK_FORMAT_R8G8B8_USCALED";
            case VK_FORMAT_R8G8B8_SSCALED:
                return "VK_FORMAT_R8G8B8_SSCALED";
            case VK_FORMAT_R8G8B8_UINT:
                return "VK_FORMAT_R8G8B8_UINT";
            case VK_FORMAT_R8G8B8_SINT:
                return "VK_FORMAT_R8G8B8_SINT";
            case VK_FORMAT_R8G8B8_SRGB:
                return "VK_FORMAT_R8G8B8_SRGB";
            case VK_FORMAT_B8G8R8_UNORM:
                return "VK_FORMAT_B8G8R8_UNORM";
            case VK_FORMAT_B8G8R8_SNORM:
                return "VK_FORMAT_B8G8R8_SNORM";
            case VK_FORMAT_B8G8R8_USCALED:
                return "VK_FORMAT_B8G8R8_USCALED";
            case VK_FORMAT_B8G8R8_SSCALED:
                return "VK_FORMAT_B8G8R8_SSCALED";
            case VK_FORMAT_B8G8R8_UINT:
                return "VK_FORMAT_B8G8R8_UINT";
            case VK_FORMAT_B8G8R8_SINT:
                return "VK_FORMAT_B8G8R8_SINT";
            case VK_FORMAT_B8G8R8_SRGB:
                return "VK_FORMAT_B8G8R8_SRGB";
            case VK_FORMAT_R8G8B8A8_UNORM:
                return "VK_FORMAT_R8G8B8A8_UNORM";
            case VK_FORMAT_R8G8B8A8_SNORM:
                return "VK_FORMAT_R8G8B8A8_SNORM";
            case VK_FORMAT_R8G8B8A8_USCALED:
                return "VK_FORMAT_R8G8B8A8_USCALED";
            case VK_FORMAT_R8G8B8A8_SSCALED:
                return "VK_FORMAT_R8G8B8A8_SSCALED";
            case VK_FORMAT_R8G8B8A8_UINT:
                return "VK_FORMAT_R8G8B8A8_UINT";
            case VK_FORMAT_R8G8B8A8_SINT:
                return "VK_FORMAT_R8G8B8A8_SINT";
            case VK_FORMAT_R8G8B8A8_SRGB:
                return "VK_FORMAT_R8G8B8A8_SRGB";
            case VK_FORMAT_B8G8R8A8_UNORM:
                return "VK_FORMAT_B8G8R8A8_UNORM";
            case VK_FORMAT_B8G8R8A8_SNORM:
                return "VK_FORMAT_B8G8R8A8_SNORM";
            case VK_FORMAT_B8G8R8A8_USCALED:
                return "VK_FORMAT_B8G8R8A8_USCALED";
            case VK_FORMAT_B8G8R8A8_SSCALED:
                return "VK_FORMAT_B8G8R8A8_SSCALED";
            case VK_FORMAT_B8G8R8A8_UINT:
                return "VK_FORMAT_B8G8R8A8_UINT";
            case VK_FORMAT_B8G8R8A8_SINT:
                return "VK_FORMAT_B8G8R8A8_SINT";
            case VK_FORMAT_B8G8R8A8_SRGB:
                return "VK_FORMAT_B8G8R8A8_SRGB";
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
                return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
                return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
                return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
                return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
                return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
                return "";
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
                return "";
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
                return "";
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
                return "";
            case VK_FORMAT_R16_UNORM:
                return "";
            case VK_FORMAT_R16_SNORM:
                return "";
            case VK_FORMAT_R16_USCALED:
                return "";
            case VK_FORMAT_R16_SSCALED:
                return "";
            case VK_FORMAT_R16_UINT:
                return "";
            case VK_FORMAT_R16_SINT:
                return "";
            case VK_FORMAT_R16_SFLOAT:
                return "";
            case VK_FORMAT_R16G16_UNORM:
                return "";
            case VK_FORMAT_R16G16_SNORM:
                return "";
            case VK_FORMAT_R16G16_USCALED:
                return "";
            case VK_FORMAT_R16G16_SSCALED:
                return "";
            case VK_FORMAT_R16G16_UINT:
                return "";
            case VK_FORMAT_R16G16_SINT:
                return "";
            case VK_FORMAT_R16G16_SFLOAT:
                return "";
            case VK_FORMAT_R16G16B16_UNORM:
                return "";
            case VK_FORMAT_R16G16B16_SNORM:
                return "";
            case VK_FORMAT_R16G16B16_USCALED:
                return "";
            case VK_FORMAT_R16G16B16_SSCALED:
                return "";
            case VK_FORMAT_R16G16B16_UINT:
                return "";
            case VK_FORMAT_R16G16B16_SINT:
                return "";
            case VK_FORMAT_R16G16B16_SFLOAT:
                return "";
            case VK_FORMAT_R16G16B16A16_UNORM:
                return "";
            case VK_FORMAT_R16G16B16A16_SNORM:
                return "";
            case VK_FORMAT_R16G16B16A16_USCALED:
                return "";
            case VK_FORMAT_R16G16B16A16_SSCALED:
                return "";
            case VK_FORMAT_R16G16B16A16_UINT:
                return "";
            case VK_FORMAT_R16G16B16A16_SINT:
                return "";
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return "";
            case VK_FORMAT_R32_UINT:
                return "";
            case VK_FORMAT_R32_SINT:
                return "";
            case VK_FORMAT_R32_SFLOAT:
                return "";
            case VK_FORMAT_R32G32_UINT:
                return "";
            case VK_FORMAT_R32G32_SINT:
                return "";
            case VK_FORMAT_R32G32_SFLOAT:
                return "";
            case VK_FORMAT_R32G32B32_UINT:
                return "";
            case VK_FORMAT_R32G32B32_SINT:
                return "";
            case VK_FORMAT_R32G32B32_SFLOAT:
                return "";
            case VK_FORMAT_R32G32B32A32_UINT:
                return "";
            case VK_FORMAT_R32G32B32A32_SINT:
                return "";
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return "";
            case VK_FORMAT_R64_UINT:
                return "";
            case VK_FORMAT_R64_SINT:
                return "";
            case VK_FORMAT_R64_SFLOAT:
                return "";
            case VK_FORMAT_R64G64_UINT:
                return "";
            case VK_FORMAT_R64G64_SINT:
                return "";
            case VK_FORMAT_R64G64_SFLOAT:
                return "";
            case VK_FORMAT_R64G64B64_UINT:
                return "";
            case VK_FORMAT_R64G64B64_SINT:
                return "";
            case VK_FORMAT_R64G64B64_SFLOAT:
                return "";
            case VK_FORMAT_R64G64B64A64_UINT:
                return "";
            case VK_FORMAT_R64G64B64A64_SINT:
                return "";
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                return "";
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
                return "";
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
                return "";
            case VK_FORMAT_D16_UNORM:
                return "";
            case VK_FORMAT_X8_D24_UNORM_PACK32:
                return "";
            case VK_FORMAT_D32_SFLOAT:
                return "";
            case VK_FORMAT_S8_UINT:
                return "";
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return "";
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return "";
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return "";
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
                return "";
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
                return "";
            case VK_FORMAT_BC2_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC2_SRGB_BLOCK:
                return "";
            case VK_FORMAT_BC3_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC3_SRGB_BLOCK:
                return "";
            case VK_FORMAT_BC4_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC4_SNORM_BLOCK:
                return "";
            case VK_FORMAT_BC5_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC5_SNORM_BLOCK:
                return "";
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
                return "";
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_BC7_UNORM_BLOCK:
                return "";
            case VK_FORMAT_BC7_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
                return "";
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:
                return "";
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:
                return "";
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
                return "";
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
                return "";
            case VK_FORMAT_G8B8G8R8_422_UNORM:
                return "";
            case VK_FORMAT_B8G8R8G8_422_UNORM:
                return "";
            case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
                return "";
            case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
                return "";
            case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
                return "";
            case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
                return "";
            case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
                return "";
            case VK_FORMAT_R10X6_UNORM_PACK16:
                return "";
            case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
                return "";
            case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
                return "";
            case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
                return "";
            case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
                return "";
            case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
                return "";
            case VK_FORMAT_R12X4_UNORM_PACK16:
                return "";
            case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
                return "";
            case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
                return "";
            case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
                return "";
            case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G16B16G16R16_422_UNORM:
                return "";
            case VK_FORMAT_B16G16R16G16_422_UNORM:
                return "";
            case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
                return "";
            case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
                return "";
            case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
                return "";
            case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
                return "";
            case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
                return "";
            case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
                return "";
            case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
                return "";
            case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
                return "";
            case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
                return "";
            case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
                return "";
            case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
                return "";
            case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
                return "";
            case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
                return "";
            case VK_FORMAT_R16G16_SFIXED5_NV:
                return "";
            case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
                return "";
            case VK_FORMAT_A8_UNORM_KHR:
                return "";
            case VK_FORMAT_MAX_ENUM:
                return "";
            default:
                return "VkFormat Specified Invalid!!!";
        }
    }
};