#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_initialize.h"

#ifdef DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    // Print debug messages
    printf("%s\n", pCallbackData->pMessage);

    // Optionally, return VK_FALSE if you want to stop Vulkan from continuing after the message.
    // VK_TRUE means the message is handled, and Vulkan can continue as usual.
    return VK_TRUE;
}
#endif

VkInstance instance;

bool initialize_vulkan(){
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "TRIEX";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "TRIEX";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;

#ifndef DEBUG
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
#else
    instanceInfo.enabledLayerCount = 1;
    const char* INFOvalidationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    instanceInfo.ppEnabledLayerNames = INFOvalidationLayers;
#endif

    const char* INFOextensions[] = {
    "VK_KHR_surface",
#ifdef _WIN32
    "VK_KHR_win32_surface",
#else
    "VK_KHR_xlib_surface",
#endif

#ifdef DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };

    instanceInfo.enabledExtensionCount = ARRAY_LEN(INFOextensions);
    instanceInfo.ppEnabledExtensionNames = INFOextensions;

    VkResult result = vkCreateInstance(&instanceInfo,NULL,&instance);

    if(result != VK_SUCCESS){
        printf("Failed to create Vulkan instance: %d\n", result);
        return false;
    }

#ifdef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    VkDebugUtilsMessengerEXT debugMessenger;

    PFN_vkCreateDebugUtilsMessengerEXT functionHandle = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    functionHandle(instance, &debugCreateInfo, NULL, &debugMessenger);
#endif
    return true;
}