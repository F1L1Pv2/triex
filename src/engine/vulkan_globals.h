#ifndef FVFX_VULKAN_GLOBALS
#define FVFX_VULKAN_GLOBALS

#include "vulkan/vulkan.h"

extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkPhysicalDeviceLimits physicalDeviceLimits;
extern VkSurfaceKHR surface;
extern VkSwapchainKHR swapchain;
extern VkFormat swapchainImageFormat;
extern VkExtent2D swapchainExtent;
extern VkQueue graphicsQueue;
extern VkQueue presentQueue;
extern VkPhysicalDeviceMemoryProperties memoryProperties;

typedef struct {
    VkImage* items;
    uint32_t count;
    size_t capacity;
} VkImages;

typedef struct {
    VkImageView* items;
    uint32_t count;
    size_t capacity;
} VkImageViews;

extern VkImages swapchainImages;
extern VkImageViews swapchainImageViews;

typedef struct {
    VkFramebuffer* items;
    size_t count;
    size_t capacity;
} VkFramebuffers;

extern VkFramebuffers framebuffers;

typedef struct{
    VkQueueFamilyProperties* items;
    uint32_t count;
    size_t capacity;
} MultipleVkQueueFamilyProperties;
extern MultipleVkQueueFamilyProperties multipleQueueFamilyProperties;

extern VkCommandPool commandPool;
extern VkCommandBuffer cmd;
extern VkRenderPass renderPass;

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    vec2 pos;
} Vertex;

extern VkPipelineLayout pipelineLayout;
extern VkPipeline pipeline;

extern VkSemaphore imageAvailableSemaphore;
extern VkSemaphore renderFinishedSemaphore;
extern VkFence inFlightFence;

#endif