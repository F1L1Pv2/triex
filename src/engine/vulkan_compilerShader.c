#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"
#include "shaderc/shaderc.h"

#include "vulkan_globals.h"
#include "vulkan_compileShader.h"

bool compileShader(char* inputText, shaderc_shader_kind shaderKind, VkShaderModule* outShader){
    shaderc_compiler_t compiler = shaderc_compiler_initialize();

    shaderc_compilation_result_t result = shaderc_compile_into_spv( compiler, inputText, strlen(inputText), shaderKind, "internalVert", "main", NULL);

    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
    if(status != shaderc_compilation_status_success){
        const char* errors = shaderc_result_get_error_message(result);
        printf("ERROR (compiling shader): \n%s\n---------------------------\n%s\n", inputText,errors);
        
        shaderc_result_release(result);
        shaderc_compiler_release(compiler);

        return false;
    }

    const char* compiledShader = shaderc_result_get_bytes(result);
    size_t compiledSize = shaderc_result_get_length(result);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {0};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = NULL;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = compiledSize;
    shaderModuleCreateInfo.pCode = (uint32_t*)compiledShader;

    VkResult vresult = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, outShader);

    shaderc_result_release(result);
    shaderc_compiler_release(compiler);

    if(vresult != VK_SUCCESS){
        printf("ERROR: Couldn't create shader module\n");
        return false;
    }

    return true;
}