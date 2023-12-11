#include "snbx/device/gpu_device_api.hpp"

#include "volk.h"

#include "vulkan_platform.hpp"
#include "snbx/platform/platform.hpp"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"

constexpr u32 VK_FRAMES_IN_FLIGHT = 2;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    Vec<VkSurfaceFormatKHR> formats{};
    Vec<VkPresentModeKHR> present_modes{};
};

struct VulkanTexture {
    VkImage image{};
    VkImageView image_view{};
    VmaAllocation allocation{};
    TextureCreation texture_creation{};
};

struct VulkanRenderPass {
    VkRenderPass      render_pass{};
    VkFramebuffer     framebuffer{};
    VkExtent2D        extent{};
    Vec<VkClearValue> clear_values{};
};

struct VulkanSwapchain {
    const SwapchainCreation swapchain_creation{};

    VkSurfaceKHR            surface_KHR{};
    VkSwapchainKHR          swapchain_KHR{};
    VkExtent2D              extent{};
    VkFormat                format{};

    VkRenderPass            render_pass{};
    Vec<VkFramebuffer>      framebuffers{};
    Vec<VkImage>            images{};
    Vec<VkImageView>        image_views{};
    Vec<VkClearValue>       clear_values{};

    u32                     image_index{};

    Array<VkSemaphore, VK_FRAMES_IN_FLIGHT> image_available_semaphores{};
    Window*                     window{};
};

struct VulkanCommands {
    VkCommandBuffer vk_command_buffer{};
};

struct VulkanContext {
    VkInstance  instance = nullptr;
    VkDevice    device = nullptr;
    VkDebugUtilsMessengerEXT        debug_messenger = nullptr;
    VkPhysicalDevice                physical_device{};
    VkPhysicalDeviceProperties      gpu_properties{};
    VmaAllocator                    allocator{};
    VkCommandPool                   command_pool{};
    VkPhysicalDeviceFeatures        device_features{};
    GPUDeviceInfo                   device_info{};
    VkDescriptorPool                descriptor_pool{};

    //queues
    Vec<VkQueueFamilyProperties>    queue_families{};

    u32                             graphics_family{U32_MAX};
    u32                             present_family{U32_MAX};
    VkQueue                         graphics_queue{};
    VkQueue                         present_queue{};

    //raytrace
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR         ray_tracing_pipeline_properties{};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR      acceleration_structure_properties_properties{};

    //Validation layers
    bool        enable_validation_layers      = true;
    const char* validation_layers[1]         = {"VK_LAYER_KHRONOS_validation"};
    bool        validation_layers_available   = false;


    //sync
    Array<VkFence, VK_FRAMES_IN_FLIGHT>             in_flight_fences{};
    Array<VkSemaphore, VK_FRAMES_IN_FLIGHT>         render_finished_semaphores{};
    Array<VulkanCommands, VK_FRAMES_IN_FLIGHT>      commands{};
    u32                                             current_frame = 0;

} vk_context;

#define SNBX_CHECK_RESULT(func) result = func; \
    if (result != GPUResult::Success) return result


inline VkBool32 VKAPI_PTR vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data_ext,
                                         void *user_data) {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::trace("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            break;
    }
    return VK_FALSE;
}

bool vk_query_instance_extensions(const Span<const char *> &required_extensions) {
    u32 extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    Vec<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    for (const auto req_extension: required_extensions) {
        bool found = false;
        for (const auto &extension: extensions) {
            if (strcmp(extension.extensionName, req_extension) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }
    return true;
}

bool vk_query_instance_extension(const char *extension) {
    return vk_query_instance_extensions(Span<const char *>(&extension, &extension + 1));
}

void vk_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = &vk_debug_callback;
}


VkResult vk_create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}

bool vk_query_layer_properties(const char **required_layers, u32 size) {
    u32 extension_count = 0;
    vkEnumerateInstanceLayerProperties(&extension_count, nullptr);
    Vec<VkLayerProperties> extensions(extension_count);
    vkEnumerateInstanceLayerProperties(&extension_count, extensions.data());

    for (int i = 0; i < size; ++i) {
        const auto req_extension = required_layers[i];
        bool found = false;
        for (const auto &layer: extensions) {
            if (strcmp(layer.layerName, req_extension) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }
    return true;
}

bool vk_query_device_extensions(const Span<VkExtensionProperties>& extensions, const char* check_for_extension) {
    for (const auto& extension: extensions) {
        if (strcmp(extension.extensionName, check_for_extension) == 0) {
            return true;
        }
    }
    return false;
}

SwapChainSupportDetails vk_query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }
    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }
    return details;
}

VkSurfaceFormatKHR vk_choose_swap_surface_format(const SwapChainSupportDetails& support_details, VkSurfaceFormatKHR desired_format) {
    for (const auto& availableFormat: support_details.formats) {
        if (availableFormat.format == desired_format.format && availableFormat.colorSpace == desired_format.colorSpace) {
            return availableFormat;
        }
    }
    return support_details.formats[0];
}

VkPresentModeKHR vk_choose_swap_present_mode(const SwapChainSupportDetails& support_details, VkPresentModeKHR desired_present_mode) {
    for (const auto& availablePresentMode: support_details.present_modes) {
        if (availablePresentMode == desired_present_mode) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vk_choose_swap_extent(const SwapChainSupportDetails& support_details, UVec2 extent) {
    if (support_details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return support_details.capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent = {extent.x, extent.y};
        actual_extent.width = std::clamp(actual_extent.width, support_details.capabilities.minImageExtent.width, support_details.capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, support_details.capabilities.minImageExtent.height, support_details.capabilities.maxImageExtent.height);
        return actual_extent;
    }
}

VkFormat vk_cast_format(const GPUFormat& textureFormat) {
    switch (textureFormat) {
        case GPUFormat::R: return VK_FORMAT_R8_UNORM;
        case GPUFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
        case GPUFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
        case GPUFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GPUFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GPUFormat::BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
        case GPUFormat::Depth: return VK_FORMAT_D32_SFLOAT; //TODO should check if format is available
        case GPUFormat::R16F: return VK_FORMAT_R16_SFLOAT;
        case GPUFormat::R32F: return VK_FORMAT_R32_SFLOAT;
        case GPUFormat::RG: return VK_FORMAT_R8G8_UNORM;
        case GPUFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
        case GPUFormat::RGB: return VK_FORMAT_R8G8B8_UNORM;
        case GPUFormat::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
        case GPUFormat::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
        case GPUFormat::Undefined:
            break;
        default:
            SNBX_ASSERT(false, "[VulkanDevice] VkFormat not found");
            return VK_FORMAT_UNDEFINED;
    }
    return {};
}

VkImageViewType vk_cast_view_type(const ViewType& view_type) {
    switch (view_type) {
        case ViewType::Type1D: return VK_IMAGE_VIEW_TYPE_1D;
        case ViewType::Type2D: return VK_IMAGE_VIEW_TYPE_2D;
        case ViewType::Type3D: return VK_IMAGE_VIEW_TYPE_3D;
        case ViewType::TypeCube: return VK_IMAGE_VIEW_TYPE_CUBE;
        case ViewType::Type1DArray: return VK_IMAGE_VIEW_TYPE_1D;
        case ViewType::Type2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case ViewType::TypeCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case ViewType::Undefined: break;
    }
    SNBX_ASSERT(false, "vulkan_utils.hpp: CastLayout not found");
    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

constexpr bool vk_is_depth_format(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT
           || format == VK_FORMAT_D32_SFLOAT
           || format == VK_FORMAT_D24_UNORM_S8_UINT
           || format == VK_FORMAT_D16_UNORM_S8_UINT
           || format == VK_FORMAT_D16_UNORM;
}

VkImageUsageFlags vk_cast_render_flags(RenderFlags render_flags) {
    VkImageUsageFlags usage{};
    if (render_flags && RenderFlags::ShaderResource) {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (render_flags && RenderFlags::DepthStencil) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (render_flags && RenderFlags::RenderPass) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (render_flags && RenderFlags::Storage) {
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (render_flags && RenderFlags::TransferDst) {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (render_flags && RenderFlags::TransferSrc) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    return usage;
}

VkImageLayout vk_cast_layout(const ResourceLayout& resource_layout, VkImageLayout default_undefined = VK_IMAGE_LAYOUT_UNDEFINED) {
    switch (resource_layout) {
        case ResourceLayout::Undefined: return default_undefined;
        case ResourceLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
        case ResourceLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ResourceLayout::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ResourceLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ResourceLayout::CopyDest: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ResourceLayout::CopySource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ResourceLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    SNBX_ASSERT(false, "vk_cast_layout not found");
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkAttachmentStoreOp vk_cast_store_op(StoreOp store_op) {
    switch (store_op) {
        case StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    SNBX_ASSERT(false, "vk_cast_store_op not found");
    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

VkAttachmentLoadOp vk_cast_load_op(LoadOp load_op) {
    switch (load_op) {
        case LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    SNBX_ASSERT(false, "vk_cast_load_op not found");
    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}


GPUResult vk_create_instance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "SNBX";
    app_info.applicationVersion = 0;
    app_info.pEngineName = "SNBX";
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    u32 validation_layer_size = sizeof(vk_context.validation_layers) / sizeof(const char*);
    vk_context.validation_layers_available = (vk_context.enable_validation_layers && vk_query_layer_properties(vk_context.validation_layers, validation_layer_size));

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};

    if (vk_context.validation_layers_available)
    {
        create_info.enabledLayerCount = validation_layer_size;
        create_info.ppEnabledLayerNames = vk_context.validation_layers;
        vk_populate_debug_messenger_create_info(debug_utils_messenger_create_info);
        create_info.pNext = &debug_utils_messenger_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

   auto platform_req = platform_get_required_instance_extensions();

    Vec<const char*> required_extensions{};
    required_extensions.insert(required_extensions.begin(), platform_req.begin(), platform_req.end());
    if (vk_context.validation_layers_available) {
        required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!vk_query_instance_extensions(required_extensions)) {
        spdlog::error("[Vulkan] Required extensions not found");
        return GPUResult::NotSupported;
    }

#ifdef SNBX_MACOS
    if (query_instance_extensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    if (vkCreateInstance(&create_info, nullptr, &vk_context.instance) != VK_SUCCESS) {
        spdlog::error("[Vulkan] error on create vkCreateInstance");
        return GPUResult::NotSupported;
    }

    return GPUResult::Success;
}

void setup_debug_messenger() {
    if (vk_context.validation_layers_available) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        vk_populate_debug_messenger_create_info(createInfo);
        if (vk_create_debug_utils_messenger_ext(vk_context.instance, &createInfo, nullptr, &vk_context.debug_messenger) != VK_SUCCESS) {
            spdlog::warn("[Vulkan] failed to set up debug messenger!");
        }
    }
}

VkPhysicalDevice vk_choose_physical_device(const Vec<VkPhysicalDevice>& devices) {
    if (devices.size() == 1) {
        return devices[0];
    }

    // if (context.renderSettings->gpuIndex != U32_MAX && devices.Size() > context.renderSettings->gpuIndex)
    // {
    //     return devices[context.renderSettings->gpuIndex];
    // }

    Vec<std::pair<u32, VkPhysicalDevice>> device_score{};
    device_score.reserve(devices.size());

    for (VkPhysicalDevice device: devices) {
        u32 score = 0;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 10000;
        }

        score += device_properties.limits.maxImageDimension2D;
        device_score.emplace_back(score, device);
        spdlog::debug("[Vulkan] device {} found", device_properties.deviceName);
    }

    std::ranges::sort(device_score, [](const std::pair<u32, VkPhysicalDevice>& r, const std::pair<u32, VkPhysicalDevice>& l) {
        return r.first > l.first;
    });
    return device_score[0].second;
}

GPUResult vk_select_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vk_context.instance, &device_count, nullptr);

    if (device_count == 0) {
        spdlog::error("failed to find GPUs with Vulkan support!");
        return GPUResult::NotSupported;
    }

    Vec<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vk_context.instance, &device_count, devices.data());

    vk_context.physical_device = vk_choose_physical_device(devices);

    vkGetPhysicalDeviceFeatures(vk_context.physical_device, &vk_context.device_features);
    vk_context.device_info.multi_draw_indirect_supported = vk_context.device_features.multiDrawIndirect;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr};
    VkPhysicalDeviceFeatures2 deviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures};
    vkGetPhysicalDeviceFeatures2(vk_context.physical_device, &deviceFeatures2);
    vk_context.device_info.bindless_supported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(vk_context.physical_device, nullptr, &extension_count, nullptr);

    Vec<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(vk_context.physical_device, nullptr, &extension_count, available_extensions.data());

    for (int i = 0; i < extension_count; ++i) {
        spdlog::debug("[Vulkan] Extension {} ", available_extensions[i].extensionName);
    }

    if (!vk_query_device_extensions(available_extensions,VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        spdlog::warn("[Vulkan] extension {} not found", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        return GPUResult::NotSupported;
    }

    //vulkanDeviceInfo.dynamicRenderingExtensionPresent = vk_query_device_extensions(availableExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    //context.vulkanDeviceInfo.maintenance4Available = vk_query_device_extensions(availableExtensions, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

    vk_context.device_info.raytrace_supported = vk_query_device_extensions(available_extensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    spdlog::debug("[Vulkan] Raytrace enabled");

    vkGetPhysicalDeviceProperties(vk_context.physical_device, &vk_context.gpu_properties);

    if (vk_context.device_info.raytrace_supported) {
        vk_context.ray_tracing_pipeline_properties = {};
        vk_context.ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        VkPhysicalDeviceProperties2 device_properties2{};
        device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties2.pNext = &vk_context.ray_tracing_pipeline_properties;
        vkGetPhysicalDeviceProperties2(vk_context.physical_device, &device_properties2);

        vk_context.acceleration_structure_properties_properties = {};
        vk_context.acceleration_structure_properties_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &vk_context.acceleration_structure_properties_properties;
        vkGetPhysicalDeviceFeatures2(vk_context.physical_device, &features2);
    }
    return GPUResult::Success;
}

void vk_update_required_queue_families() {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.physical_device, &queueFamilyCount, nullptr);
    vk_context.queue_families.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.physical_device, &queueFamilyCount, vk_context.queue_families.data());

    int i = 0;
    for (const auto& queue_family: vk_context.queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk_context.graphics_family = i;
        }

        //try to keep graphics and present in the same queue
        if (platform_get_physical_device_presentation_support(vk_context.instance, vk_context.physical_device, i)) {
            vk_context.present_family = i;
        }

        if (vk_context.graphics_family != U32_MAX && vk_context.present_family != U32_MAX) {
            break;
        }
        i++;
    }
}

GPUResult vk_create_logical_device() {
    f32 queuePriority = 1.0f;

    //**raytrace***
    VkPhysicalDeviceRayQueryFeaturesKHR device_ray_query_features_khr{};
    device_ray_query_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    device_ray_query_features_khr.pNext = nullptr;
    device_ray_query_features_khr.rayQuery = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR device_acceleration_structure_features_khr{};

    device_acceleration_structure_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    device_acceleration_structure_features_khr.pNext = &device_ray_query_features_khr;
    device_acceleration_structure_features_khr.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeatures{};
    deviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    deviceRayTracingPipelineFeatures.pNext = &device_acceleration_structure_features_khr;
    deviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

    //endraytrace.
    Vec<VkDeviceQueueCreateInfo> queue_create_infos{};
    if (vk_context.graphics_family != vk_context.present_family) {
        queue_create_infos.resize(2);

        queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].queueFamilyIndex = vk_context.graphics_family;
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queuePriority;

        queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[1].queueFamilyIndex = vk_context.present_family;
        queue_create_infos[1].queueCount = 1;
        queue_create_infos[1].pQueuePriorities = &queuePriority;
    } else {
        queue_create_infos.resize(1);
        queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].queueFamilyIndex = vk_context.graphics_family;
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queuePriority;
    }
    VkPhysicalDeviceFeatures physical_device_features{};

    if (vk_context.device_features.samplerAnisotropy) {
        physical_device_features.samplerAnisotropy = VK_TRUE;
    }

    if (vk_context.device_info.multi_draw_indirect_supported) {
        physical_device_features.multiDrawIndirect = VK_TRUE;
    }

    physical_device_features.shaderInt64 = VK_TRUE;
    physical_device_features.fillModeNonSolid = true;

    VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress = VK_TRUE;


    VkPhysicalDeviceMaintenance4FeaturesKHR vk_physical_device_maintenance4_features_khr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
    vk_physical_device_maintenance4_features_khr.maintenance4 = true;

    if (vk_context.device_info.raytrace_supported) {
        vk_physical_device_maintenance4_features_khr.pNext = &deviceRayTracingPipelineFeatures;
    }

    // if (context.vulkanDeviceInfo.maintenance4Available) {
    // 	features12.pNext = &vkPhysicalDeviceMaintenance4FeaturesKhr;
    // }

    if (vk_context.device_info.bindless_supported) {
        features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        features12.runtimeDescriptorArray = VK_TRUE;
        features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        features12.descriptorBindingPartiallyBound = VK_TRUE;
        features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    }

    VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = queue_create_infos.size();
    create_info.pEnabledFeatures = &physical_device_features;

    Vec<const char *> deviceExtensions{};
    deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


    // if (context.vulkanDeviceInfo.maintenance4Available) {
    // 	deviceExtensions.emplace_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    // }

    if (vk_context.device_info.raytrace_supported) {
        deviceExtensions.emplace_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    }

    // if (context.vulkanDeviceInfo.dynamicRenderingExtensionPresent) {
    // 	deviceExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    // }

#ifdef SNBX_MACOS
		deviceExtensions.emplace_back("VK_KHR_portability_subset");
#endif

    create_info.enabledExtensionCount = static_cast<u32>(deviceExtensions.size());
    create_info.ppEnabledExtensionNames = deviceExtensions.data();

    if (vk_context.validation_layers_available) {
        auto validationLayerSize = sizeof(vk_context.validation_layers) / sizeof(const char *);
        create_info.enabledLayerCount = validationLayerSize;
        create_info.ppEnabledLayerNames = vk_context.validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
    }

    // VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR};
    // if (context.vulkanDeviceInfo.dynamicRenderingExtensionPresent) {
    //     dynamicRenderingFeatures.dynamicRendering = true;
    //     dynamicRenderingFeatures.pNext = features12.pNext;
    //     features12.pNext = &dynamicRenderingFeatures;
    //     context.logger.Debug("Dynamic Rendering enabled");
    // }

    create_info.pNext = &features12;
    if (vkCreateDevice(vk_context.physical_device, &create_info, nullptr, &vk_context.device) != VK_SUCCESS) {
        spdlog::error("failed to create device");
        return GPUResult::NotSupported;
    }

    return GPUResult::Success;
}


GPUResult vk_init() {
    GPUResult result;

    platform_init_vk();
    volkInitialize();
    SNBX_CHECK_RESULT(vk_create_instance());
    setup_debug_messenger();
    volkLoadInstance(vk_context.instance);
    SNBX_CHECK_RESULT(vk_select_physical_device());
    vk_update_required_queue_families();
    SNBX_CHECK_RESULT(vk_create_logical_device());

    VmaVulkanFunctions vma_vulkan_functions{};
    vma_vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo vma_allocator_create_info = {};
    vma_allocator_create_info.physicalDevice = vk_context.physical_device;
    vma_allocator_create_info.device = vk_context.device;
    vma_allocator_create_info.instance = vk_context.instance;
    vma_allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vma_allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
    vmaCreateAllocator(&vma_allocator_create_info, &vk_context.allocator);

    VkCommandPoolCreateInfo command_pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = vk_context.graphics_family;
    vkCreateCommandPool(vk_context.device, &command_pool_info, nullptr, &vk_context.command_pool);


    VkCommandBufferAllocateInfo temp_alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    temp_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    temp_alloc_info.commandPool = vk_context.command_pool;
    temp_alloc_info.commandBufferCount = 1;

    // VulkanCommands* vulkanCommands = new VulkanCommands{};
    // vkAllocateCommandBuffers(context.device, &temp_alloc_info, &vulkanCommands->commandBuffer);
    // vkGetDeviceQueue(context.device, context.graphicsFamily, 0, &vulkanCommands->queue);
    // context.singleTimeCommands.handler = vulkanCommands;

    VkDescriptorPoolSize sizes[4] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 500},
        {VK_DESCRIPTOR_TYPE_SAMPLER, 500},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 500},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 500}
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 4;
    pool_info.pPoolSizes = sizes;
    pool_info.maxSets = 500;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vk_context.device_info.bindless_supported) {
        pool_info.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    }

    vkCreateDescriptorPool(vk_context.device, &pool_info, nullptr, &vk_context.descriptor_pool);
    vkGetDeviceQueue(vk_context.device, vk_context.graphics_family, 0, &vk_context.graphics_queue);
    vkGetDeviceQueue(vk_context.device, vk_context.present_family, 0, &vk_context.present_queue);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = vk_context.command_pool;
    alloc_info.commandBufferCount = 1;

    for (int i = 0; i < VK_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(vk_context.device, &semaphore_info, nullptr, &vk_context.render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vk_context.device, &fence_info, nullptr, &vk_context.in_flight_fences[i]) != VK_SUCCESS ||
            vkAllocateCommandBuffers(vk_context.device, &alloc_info, &vk_context.commands[i].vk_command_buffer) != VK_SUCCESS) {
            spdlog::error("Failed to create frame");
            return GPUResult::NotSupported;
        }
    }

    spdlog::info("Vulkan API {}.{}.{} Device: {} ",
                 VK_VERSION_MAJOR(vk_context.gpu_properties.apiVersion),
                 VK_VERSION_MINOR(vk_context.gpu_properties.apiVersion),
                 VK_VERSION_PATCH(vk_context.gpu_properties.apiVersion),
                 vk_context.gpu_properties.deviceName);

    return result;
}

GPUSwapchain vk_create_swapchain(VulkanSwapchain* vulkan_swapchain) {
    VkResult res = platform_create_window_surface(vulkan_swapchain->window, vk_context.instance, &vulkan_swapchain->surface_KHR);

    if (res != VK_SUCCESS) {
        spdlog::error("swapchain surface cannot be created");
        return {};
    }

    SwapChainSupportDetails details = vk_query_swapchain_support(vk_context.physical_device, vulkan_swapchain->surface_KHR);
    VkSurfaceFormatKHR format = vk_choose_swap_surface_format(details, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
    VkPresentModeKHR present_mode = vk_choose_swap_present_mode(details, vulkan_swapchain->swapchain_creation.vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR);

    UVec2 win_size = platform_window_get_size(vulkan_swapchain->window);

    vulkan_swapchain->extent = vk_choose_swap_extent(details, {win_size.x, win_size.y});

    u32 image_count = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vk_context.physical_device,
                                         vk_context.present_family,
                                         vulkan_swapchain->surface_KHR,
                                         &present_support);


    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vulkan_swapchain->surface_KHR;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = vulkan_swapchain->extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queue_family_indices[] = {vk_context.graphics_family, vk_context.present_family};
    if (vk_context.graphics_family != vk_context.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(vk_context.device, &create_info, nullptr, &vulkan_swapchain->swapchain_KHR);
    vkGetSwapchainImagesKHR(vk_context.device, vulkan_swapchain->swapchain_KHR, &image_count, nullptr);

    vulkan_swapchain->format = format.format;

    vulkan_swapchain->images.resize(image_count);
    vulkan_swapchain->image_views.resize(image_count);
    vulkan_swapchain->framebuffers.resize(image_count);

    vkGetSwapchainImagesKHR(vk_context.device, vulkan_swapchain->swapchain_KHR, &image_count, vulkan_swapchain->images.data());

    for (size_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vulkan_swapchain->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = format.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vk_context.device, &imageViewCreateInfo, nullptr, &vulkan_swapchain->image_views[i]);
    }

    VkAttachmentDescription attachment_description{};
    VkAttachmentReference color_attachment_reference{};

    attachment_description.format = format.format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_reference.attachment = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment_description;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 0;
    vkCreateRenderPass(vk_context.device, &render_pass_info, nullptr, &vulkan_swapchain->render_pass);

    vulkan_swapchain->clear_values.resize(render_pass_info.attachmentCount);

    for (usize i = 0; i < image_count; i++) {
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = vulkan_swapchain->render_pass;
        framebufferCreateInfo.width = vulkan_swapchain->extent.width;
        framebufferCreateInfo.height = vulkan_swapchain->extent.height;
        framebufferCreateInfo.layers = 1u;

        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &vulkan_swapchain->image_views[i];

        vkCreateFramebuffer(vk_context.device, &framebufferCreateInfo, nullptr, &vulkan_swapchain->framebuffers[i]);
    }

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (int i = 0; i < VK_FRAMES_IN_FLIGHT; ++i) {
        vkCreateSemaphore(vk_context.device, &semaphore_info, nullptr, &vulkan_swapchain->image_available_semaphores[i]);
    }

    return {vulkan_swapchain};
}

GPUSwapchain vk_create_swapchain(const SwapchainCreation& swapchain_creation, Window* window) {
    VulkanSwapchain* vulkanSwapchain = new VulkanSwapchain{
        .swapchain_creation = swapchain_creation,
        .window = window
    };
    return {vk_create_swapchain(vulkanSwapchain)};
}

void vk_destroy_swapchain(VulkanSwapchain* vulkan_swapchain) {
    for (int i = 0; i < vulkan_swapchain->images.size(); ++i) {
        vkDestroyFramebuffer(vk_context.device, vulkan_swapchain->framebuffers[i], nullptr);
        vkDestroyImageView(vk_context.device, vulkan_swapchain->image_views[i], nullptr);
    }

    vkDestroyRenderPass(vk_context.device, vulkan_swapchain->render_pass, nullptr);
    vkDestroySwapchainKHR(vk_context.device, vulkan_swapchain->swapchain_KHR, nullptr);
    vkDestroySurfaceKHR(vk_context.instance, vulkan_swapchain->surface_KHR, nullptr);

    for (int i = 0; i < VK_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(vk_context.device, vulkan_swapchain->image_available_semaphores[i], nullptr);
    }
}

void vk_destroy_swapchain(const GPUSwapchain& swapchain) {
    VulkanSwapchain* vulkan_swapchain = static_cast<VulkanSwapchain*>(swapchain.handler);
    vk_destroy_swapchain(vulkan_swapchain);
    delete vulkan_swapchain;
}

void vk_recreate_swapchain(VulkanSwapchain* vulkan_swapchain) {
    vkDeviceWaitIdle(vk_context.device);
    vk_destroy_swapchain(vulkan_swapchain);
    vk_create_swapchain(vulkan_swapchain);
}

GPUPipelineState vk_create_graphics_pipeline(const GraphicsPipelineCreation& graphics_pipeline_creation) {
    return {};
}

GPURenderPass vk_create_render_pass(const RenderPassCreation& render_pass_creation) {
    VulkanRenderPass* vulkan_render_pass = new VulkanRenderPass;

    Vec<VkAttachmentDescription> attachment_descriptions{};
    Vec<VkAttachmentReference> color_attachment_reference{};
    Vec<VkImageView> image_views{};
    VkAttachmentReference depth_reference{};

    bool has_depth = false;
    UVec3 framebuffer_size{};

    vulkan_render_pass->clear_values.resize(render_pass_creation.attachments.size());

    for (int i = 0; i < render_pass_creation.attachments.size(); ++i) {
        const AttachmentCreation& attachment = render_pass_creation.attachments[i];

        VulkanTexture* vulkan_texture = static_cast<VulkanTexture *>(attachment.texture.handler);
        SNBX_ASSERT(vulkan_texture, "texture is mandatory");

        image_views.emplace_back(vulkan_texture->image_view);

        VkFormat format = vk_cast_format(vulkan_texture->texture_creation.format);
        framebuffer_size = vulkan_texture->texture_creation.extent;
        bool is_depth_format = vk_is_depth_format(format);

        VkAttachmentDescription attachment_description{};
        attachment_description.format = format;
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;

        attachment_description.loadOp = vk_cast_load_op(attachment.load_op);
        attachment_description.storeOp = vk_cast_store_op(attachment.store_op);

        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachment_description.initialLayout = vk_cast_layout(attachment.initial_layout);

        if (!is_depth_format) {
            attachment_description.finalLayout = vk_cast_layout(attachment.final_layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            VkAttachmentReference reference{};
            reference.attachment = i;
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_attachment_reference.emplace_back(reference);
        } else {
            attachment_description.finalLayout = vk_cast_layout(attachment.final_layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            depth_reference.attachment = i;
            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            has_depth = true;
        }
        attachment_descriptions.emplace_back(attachment_description);
    }

    vulkan_render_pass->extent = {framebuffer_size.x, framebuffer_size.y};

    VkSubpassDescription sub_pass = {};
    sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub_pass.colorAttachmentCount = color_attachment_reference.size();
    sub_pass.pColorAttachments = color_attachment_reference.data();
    if (has_depth) {
        sub_pass.pDepthStencilAttachment = &depth_reference;
    }

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = attachment_descriptions.size();
    render_pass_info.pAttachments = attachment_descriptions.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &sub_pass;
    render_pass_info.dependencyCount = 0;
    vkCreateRenderPass(vk_context.device, &render_pass_info, nullptr, &vulkan_render_pass->render_pass);


    VkFramebufferCreateInfo framebuffer_create_info{};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = vulkan_render_pass->render_pass;
    framebuffer_create_info.width = framebuffer_size.x;
    framebuffer_create_info.height = framebuffer_size.y;
    framebuffer_create_info.layers = std::max(framebuffer_size.z, 1u);
    framebuffer_create_info.attachmentCount = image_views.size();
    framebuffer_create_info.pAttachments = image_views.data();

    vkCreateFramebuffer(vk_context.device, &framebuffer_create_info, nullptr, &vulkan_render_pass->framebuffer);

    return {vulkan_render_pass};
}

GPUTexture vk_create_texture(const TextureCreation& texture_creation) {
    VulkanTexture* vulkan_texture = new VulkanTexture;
    vulkan_texture->texture_creation = texture_creation;
    SNBX_ASSERT(texture_creation.extent.x != 0 && texture_creation.extent.y != 0, "image sized 0 is not allowed by vulkan");

    VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = vk_cast_format(texture_creation.format);

    image_create_info.extent = {texture_creation.extent.x, texture_creation.extent.y, std::max(texture_creation.extent.z, 1u)};
    image_create_info.mipLevels = texture_creation.mip_levels;
    image_create_info.arrayLayers = texture_creation.array_layers;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    if (image_create_info.arrayLayers % 6 == 0) {
        image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    RenderFlags render_flags = texture_creation.render_flags;
    if (render_flags == RenderFlags::None) {
        render_flags = RenderFlags::TransferDst | RenderFlags::ShaderResource;
    }
    image_create_info.usage = vk_cast_render_flags(render_flags);

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    vmaCreateImage(vk_context.allocator, &image_create_info, &alloc_info, &vulkan_texture->image, &vulkan_texture->allocation, nullptr);

    VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.image = vulkan_texture->image;
    view_create_info.format = vk_cast_format(texture_creation.format);
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.levelCount = texture_creation.mip_levels;
    view_create_info.subresourceRange.layerCount = texture_creation.array_layers;

    if (image_create_info.arrayLayers % 6 == 0) {
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    } else if (texture_creation.view_type != ViewType::Undefined) {
        view_create_info.viewType = vk_cast_view_type(texture_creation.view_type);
    } else {
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }

    if (vk_is_depth_format(view_create_info.format)) {
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else {
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCreateImageView(vk_context.device, &view_create_info, nullptr, &vulkan_texture->image_view);

    return {vulkan_texture};
}

void vk_destroy_graphics_pipeline(const GPUPipelineState& gpu_pipeline_state) {
}

void vk_destroy_render_pass(const GPURenderPass& gpu_render_pass) {
    if (!gpu_render_pass.handler) return;
    VulkanRenderPass* vulkan_render_pass = static_cast<VulkanRenderPass *>(gpu_render_pass.handler);
    vkDestroyFramebuffer(vk_context.device, vulkan_render_pass->framebuffer, nullptr);
    vkDestroyRenderPass(vk_context.device, vulkan_render_pass->render_pass, nullptr);
    delete vulkan_render_pass;
}

void vk_destroy_texture(const GPUTexture& gpu_texture) {
    VulkanTexture* vulkan_texture = static_cast<VulkanTexture *>(gpu_texture.handler);
    if (vulkan_texture->image_view) {
        vkDestroyImageView(vk_context.device, vulkan_texture->image_view, nullptr);
    }
    if (vulkan_texture->allocation) {
        vmaDestroyImage(vk_context.allocator, vulkan_texture->image, vulkan_texture->allocation);
    }
    delete vulkan_texture;
}


GPUCommands vk_begin_frame() {
    vkWaitForFences(vk_context.device, 1, &vk_context.in_flight_fences[vk_context.current_frame], VK_TRUE, UINT64_MAX);
    vkResetFences(vk_context.device, 1, &vk_context.in_flight_fences[vk_context.current_frame]);

    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(vk_context.commands[vk_context.current_frame].vk_command_buffer, &begin_info);

    return {&vk_context.commands[vk_context.current_frame]};
}

void vk_begin_render_pass(const GPUCommands& cmd, const BeginRenderPassInfo& begin_render_pass_info) {
    if (begin_render_pass_info.swapchain) {

        VulkanSwapchain* vulkan_swapchain = static_cast<VulkanSwapchain *>(begin_render_pass_info.swapchain.handler);

        UVec2 size = platform_window_get_size(vulkan_swapchain->window);
        if (size.x != vulkan_swapchain->extent.width || size.y != vulkan_swapchain->extent.height) {
            while (size.x == 0 || size.y == 0) {
                size = platform_window_get_size(vulkan_swapchain->window);
                platform_wait_events();
            }
            vk_recreate_swapchain(vulkan_swapchain);
        }

        VkResult result = vkAcquireNextImageKHR(vk_context.device,
                                                vulkan_swapchain->swapchain_KHR,
                                                UINT64_MAX,
                                                vulkan_swapchain->image_available_semaphores[vk_context.current_frame],
                                                VK_NULL_HANDLE,
                                                &vulkan_swapchain->image_index);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            SNBX_ASSERT(false, "failed to acquire swap chain image!");
        }

        VulkanCommands* vulkan_commands = static_cast<VulkanCommands *>(cmd.handler);

        VkRenderPassBeginInfo render_pass_begin_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        render_pass_begin_info.renderPass = vulkan_swapchain->render_pass;
        render_pass_begin_info.framebuffer = vulkan_swapchain->framebuffers[vulkan_swapchain->image_index];
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = {size.x, size.y};

        for (int i = 0; i < begin_render_pass_info.clear_values.size(); ++i) {
            const Vec4& c = begin_render_pass_info.clear_values[i];
            vulkan_swapchain->clear_values[i].color = {c.r, c.g, c.b, c.a};
        }

        render_pass_begin_info.clearValueCount = vulkan_swapchain->clear_values.size();
        render_pass_begin_info.pClearValues = vulkan_swapchain->clear_values.data();
        vkCmdBeginRenderPass(vulkan_commands->vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    } else if (begin_render_pass_info.render_pass) {
        VulkanCommands* vulkan_commands = static_cast<VulkanCommands *>(cmd.handler);
        VulkanRenderPass* vulkan_render_pass = static_cast<VulkanRenderPass *>(begin_render_pass_info.render_pass.handler);

        VkRenderPassBeginInfo render_pass_begin_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        render_pass_begin_info.renderPass = vulkan_render_pass->render_pass;
        render_pass_begin_info.framebuffer = vulkan_render_pass->framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = {begin_render_pass_info.extent.x, begin_render_pass_info.extent.y};

        for (int i = 0; i < begin_render_pass_info.clear_values.size(); ++i) {
            const Vec4& c = begin_render_pass_info.clear_values[i];
            vulkan_render_pass->clear_values[i].color = {c.r, c.g, c.b, c.a};
        }

        if (vulkan_render_pass->clear_values.size() > begin_render_pass_info.clear_values.size()) {
            vulkan_render_pass->clear_values[vulkan_render_pass->clear_values.size() - 1].depthStencil = {
                begin_render_pass_info.depth_stencil.x, static_cast<u32>(begin_render_pass_info.depth_stencil.y)
            };
        }

        render_pass_begin_info.clearValueCount = vulkan_render_pass->clear_values.size();
        render_pass_begin_info.pClearValues = vulkan_render_pass->clear_values.data();
        vkCmdBeginRenderPass(vulkan_commands->vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }
}

void vk_end_render_pass(const GPUCommands& cmd) {
    vkCmdEndRenderPass(static_cast<VulkanCommands*>(cmd.handler)->vk_command_buffer);
}

void vk_set_viewport(const GPUCommands& cmd, const ViewportInfo& viewport_info) {
    VkViewport vk_viewport;
    vk_viewport.x = viewport_info.x;
    vk_viewport.y = viewport_info.y;
    vk_viewport.width = viewport_info.width;
    vk_viewport.height = viewport_info.height;
    vk_viewport.minDepth = viewport_info.min_depth;
    vk_viewport.maxDepth = viewport_info.max_depth;
    vkCmdSetViewport(static_cast<VulkanCommands*>(cmd.handler)->vk_command_buffer, 0, 1, &vk_viewport);
}

void vk_set_scissor(const GPUCommands& cmd, const Rect& rect) {
    VkRect2D rect2D;
    rect2D.offset.x = static_cast<i32>(rect.x);
    rect2D.offset.y = static_cast<i32>(rect.y);
    rect2D.extent.width = static_cast<u32>(rect.width);
    rect2D.extent.height = static_cast<u32>(rect.height);
    vkCmdSetScissor(static_cast<VulkanCommands*>(cmd.handler)->vk_command_buffer, 0, 1, &rect2D);
}

void vk_end_frame(const GPUSwapchain& swapchain) {
    VulkanSwapchain* vulkan_swapchain = static_cast<VulkanSwapchain *>(swapchain.handler);

    vkEndCommandBuffer(vk_context.commands[vk_context.current_frame].vk_command_buffer);
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &vulkan_swapchain->image_available_semaphores[vk_context.current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vk_context.render_finished_semaphores[vk_context.current_frame];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_context.commands[vk_context.current_frame].vk_command_buffer;
    submit_info.pWaitDstStageMask = wait_stages;

    if (vkQueueSubmit(vk_context.graphics_queue, 1, &submit_info, vk_context.in_flight_fences[vk_context.current_frame]) != VK_SUCCESS) {
        SNBX_ASSERT(false, "Failed to execute vkQueueSubmit");
    }

    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_context.render_finished_semaphores[vk_context.current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vulkan_swapchain->swapchain_KHR;
    present_info.pImageIndices = &vulkan_swapchain->image_index;

    if (vkQueuePresentKHR(vk_context.present_queue, &present_info) != VK_SUCCESS) {
        SNBX_ASSERT(false, "Failed to execute vkQueuePresentKHR");
    }

    vk_context.current_frame = (vk_context.current_frame + 1) % VK_FRAMES_IN_FLIGHT;
}

void vk_wait() {
    vkDeviceWaitIdle(vk_context.device);
}

void vk_shutdown() {

    for (size_t i = 0; i < VK_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vk_context.device, vk_context.render_finished_semaphores[i], nullptr);
        vkDestroyFence(vk_context.device, vk_context.in_flight_fences[i], nullptr);
    }

    vkDestroyDescriptorPool(vk_context.device, vk_context.descriptor_pool, nullptr);
    vkDestroyCommandPool(vk_context.device, vk_context.command_pool, nullptr);
    vmaDestroyAllocator(vk_context.allocator);

    vkDestroyDevice(vk_context.device, nullptr);
    destroy_debug_utils_messenger_ext(vk_context.instance, vk_context.debug_messenger, nullptr);
    vkDestroyInstance(vk_context.instance, nullptr);
}

void vulkan_device_register(GPUDeviceAPI& gpu_device_api) {
    gpu_device_api.init = vk_init;
    gpu_device_api.shutdown = vk_shutdown;
    gpu_device_api.create_swapchain = vk_create_swapchain;
    gpu_device_api.create_graphics_pipeline = vk_create_graphics_pipeline;
    gpu_device_api.create_render_pass = vk_create_render_pass;
    gpu_device_api.create_texture = vk_create_texture;
    gpu_device_api.begin_render_pass = vk_begin_render_pass;
    gpu_device_api.end_render_pass = vk_end_render_pass;
    gpu_device_api.set_viewport = vk_set_viewport;
    gpu_device_api.set_scissor = vk_set_scissor;
    gpu_device_api.begin_frame = vk_begin_frame;
    gpu_device_api.end_frame = vk_end_frame;
    gpu_device_api.wait = vk_wait;
    gpu_device_api.destroy_swapchain = vk_destroy_swapchain;
    gpu_device_api.destroy_graphics_pipeline = vk_destroy_graphics_pipeline;
    gpu_device_api.destroy_render_pass = vk_destroy_render_pass;
    gpu_device_api.destroy_texture = vk_destroy_texture;
}
