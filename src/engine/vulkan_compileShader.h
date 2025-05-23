#ifndef TRIEX_VULKAN_COMPILE_SHADER
#define TRIEX_VULKAN_COMPILE_SHADER

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "shaderc/shaderc.h"

bool compileShader(char* inputText, shaderc_shader_kind shaderKind, VkShaderModule* outShader);

#endif