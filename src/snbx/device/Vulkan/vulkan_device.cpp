#include "snbx/device/gpu_device_api.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"
#include "vulkan_platform.hpp"

struct VulkanContext {
    VkInstance  instance = nullptr;
    VkDevice    device = nullptr;
    VkDebugUtilsMessengerEXT        debug_messenger = nullptr;

    //Validation layers
    bool        enable_validation_layers      = true;
    const char* validation_layers[1]         = {"VK_LAYER_KHRONOS_validation"};
    bool        validation_layers_available   = false;
} context;


inline VkBool32 VKAPI_PTR debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
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

bool query_instance_extensions(const Span<const char *> &required_extensions) {
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

bool query_instance_extension(const char *extension) {
    return query_instance_extensions(Span<const char *>(&extension, &extension + 1));
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = &debug_callback;
}


VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
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

bool query_layer_properties(const char **required_layers, u32 size) {
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

void vk_create_instance() {
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


    auto validation_layer_size = sizeof(context.validation_layers) / sizeof(const char*);
    context.validation_layers_available = (context.enable_validation_layers && query_layer_properties(context.validation_layers, validation_layer_size));

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};

    if (context.validation_layers_available)
    {
        create_info.enabledLayerCount = validation_layer_size;
        create_info.ppEnabledLayerNames = context.validation_layers;
        populate_debug_messenger_create_info(debug_utils_messenger_create_info);
        create_info.pNext = &debug_utils_messenger_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

   auto platform_req = platform_get_required_instance_extensions();

    Vec<const char*> required_extensions{};
    required_extensions.insert(required_extensions.begin(), platform_req.begin(), platform_req.end());
    if (context.validation_layers_available) {
        required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!query_instance_extensions(required_extensions)) {
        spdlog::error("[Vulkan] Required extensions not found");
        SNBX_ASSERT(false, "[Vulkan] Required extensions not found");
    }

#ifdef SNBX_MACOS
    if (query_instance_extensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    if (vkCreateInstance(&create_info, nullptr, &context.instance) != VK_SUCCESS) {
        spdlog::error("[Vulkan] error on create vkCreateInstance");
        SNBX_ASSERT(false, "[Vulkan] error on create vkCreateInstance");
    }
}

void setup_debug_messenger() {
    if (context.validation_layers_available) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populate_debug_messenger_create_info(createInfo);
        if (create_debug_utils_messenger_ext(context.instance, &createInfo, nullptr, &context.debug_messenger) != VK_SUCCESS) {
            spdlog::warn("[Vulkan] failed to set up debug messenger!");
            return;
        }
    }
}


void vk_init() {
    platform_init_vk();
    volkInitialize();
    vk_create_instance();

    setup_debug_messenger();
    volkLoadInstance(context.instance);
}

GPUSwapchain vk_create_swapchain(const SwapChainCreation& swapchain_creation, Window* window) {
    return {};
}

void vk_destroy_swapchain(const GPUSwapchain& swapchain) {

}

void vk_shutdown() {

}

void vulkan_device_register(GPUDeviceAPI& gpu_device_api) {
    gpu_device_api.init = vk_init;
    gpu_device_api.shutdown = vk_shutdown;
    gpu_device_api.create_swapchain = vk_create_swapchain;
    gpu_device_api.destroy_swapchain = vk_destroy_swapchain;
}