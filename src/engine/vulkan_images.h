#ifndef TRIEX_VULKAN_IMAGES
#define TRIEX_VULKAN_IMAGES

#include <stdbool.h>
#include <vulkan/vulkan.h>

bool createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* out);

#endif