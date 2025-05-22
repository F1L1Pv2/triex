#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_initSwapchain.h"
#include "vulkan_images.h"

VkSwapchainKHR swapchain;
VkFormat swapchainImageFormat;
VkExtent2D swapchainExtent;
VkImages swapchainImages;
VkImageViews swapchainImageViews;

typedef struct {
    VkSurfaceFormatKHR* items;
    uint32_t count;
    size_t capacity;
} VkSurfaceFormats;

uint32_t clamp_uint32(uint32_t a, uint32_t min, uint32_t max){
    if(a < min) return min;
    if(a > max) return max;
    return a;
}

bool initSwapchain(){
    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {0};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.pNext = NULL;
    swapchainCreateInfoKHR.flags = 0;
    swapchainCreateInfoKHR.surface = surface;

    VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilitiesKHR);

    VkSurfaceFormats surfaceFormats = {0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormats.count, NULL);
    da_resize(&surfaceFormats, surfaceFormats.count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormats.count, surfaceFormats.items);

    VkSurfaceFormatKHR surfaceFormat = surfaceFormats.items[0];
    bool foundFormat = false;
    for(size_t i = 0; i < surfaceFormats.count; i++){
        VkSurfaceFormatKHR availableFormat = surfaceFormats.items[i];
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            surfaceFormat = availableFormat;
            foundFormat = true;
            break;
        }
    }

    if(foundFormat == false){
        printf("ERROR: Couldn't find needed image format\n");
        da_free(surfaceFormats);
        return false;
    }

    if(surfaceCapabilitiesKHR.currentExtent.width == UINT32_MAX){
        VkExtent2D extent = {
            clamp_uint32(640,surfaceCapabilitiesKHR.minImageExtent.width,surfaceCapabilitiesKHR.maxImageExtent.width),
            clamp_uint32(480,surfaceCapabilitiesKHR.minImageExtent.height,surfaceCapabilitiesKHR.maxImageExtent.height),
        };
        swapchainCreateInfoKHR.imageExtent = extent;
    }else{
        swapchainCreateInfoKHR.imageExtent = surfaceCapabilitiesKHR.currentExtent;
    }
    swapchainExtent = swapchainCreateInfoKHR.imageExtent;

    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t imageCount = surfaceCapabilitiesKHR.minImageCount + 1;
    if(surfaceCapabilitiesKHR.maxImageCount > 0 && imageCount > surfaceCapabilitiesKHR.maxImageCount){
        imageCount = surfaceCapabilitiesKHR.maxImageCount;
    }

    swapchainCreateInfoKHR.minImageCount = imageCount;
    swapchainCreateInfoKHR.imageFormat = surfaceFormat.format;
    swapchainImageFormat = surfaceFormat.format;
    swapchainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.imageArrayLayers = 1;

    VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, NULL, &swapchain);

    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't create swapchain\n");
        da_free(surfaceFormats);
        return false;
    }

    da_free(surfaceFormats);


    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImages.count,NULL);
    if(swapchainImages.count == 0) {
        printf("ERROR: Couldn't get swapchain images\n");
        return false;
    }

    da_resize(&swapchainImages,swapchainImages.count);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImages.count,swapchainImages.items);

    da_resize(&swapchainImageViews,swapchainImages.count);

    for(int i = 0; i < swapchainImageViews.count; i++){
        if(!createImageView(swapchainImages.items[i],swapchainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT,&swapchainImageViews.items[i])) return false;
    }

    return true;
}