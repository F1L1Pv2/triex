#ifndef TRIEX_VULKAN_INIT_GRAPHICS_PIPELINE
#define TRIEX_VULKAN_INIT_GRAPHICS_PIPELINE

#include <stdbool.h>
#include <vulkan/vulkan.h>

bool initGraphicsPipeline(VkShaderModule vertexShader, VkShaderModule fragmentShader);

#endif