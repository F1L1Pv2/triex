#ifndef FVFX_VULKAN_HELPERS
#define FVFX_VULKAN_HELPERS

#include <stdbool.h>

#include "vulkan/vulkan.h"
int getNeededQueueFamilyIndex(VkQueueFlags flags);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

#endif