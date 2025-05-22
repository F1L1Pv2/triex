#ifndef FVFX_VULKAN_IMAGES
#define FVFX_VULKAN_IMAGES

#include <stdbool.h>
#include <vulkan/vulkan.h>

bool createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* out);

#endif