#include <stdio.h>
#include <stdint.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "engine/vulkan_globals.h"
#include "engine/vulkan_initialize.h"
#include "engine/vulkan_getDevice.h"
#include "engine/vulkan_createSurface.h"
#include "engine/vulkan_initSwapchain.h"
#include "engine/vulkan_initCommandPool.h"
#include "engine/vulkan_initCommandBuffer.h"
#include "engine/vulkan_initRenderPass.h"
#include "engine/vulkan_initFramebuffers.h"
#include "engine/vulkan_initPipelineLayout.h"
#include "engine/vulkan_compileShader.h"
#include "engine/vulkan_initGraphicsPipeline.h"
#include "engine/vulkan_synchro.h"
#include "engine/vulkan_helpers.h"
#include "engine/platform.h"

int main(){
    printf("hello from C\nVULKANO PREPARE!\n");

    platform_create_window("FVFX",640,480);

    if(!initialize_vulkan()) return 1;
    if(!createSurface()) return 1;
    if(!getDevice()) return 1;
    if(!initSwapchain()) return 1;
    if(!initRenderPass()) return 1;
    if(!initFramebuffers()) return 1;
    if(!initPipelineLayout()) return 1;
    if(!initCommandPool()) return 1;
    if(!initCommandBuffer()) return 1;
    if(!createAllNeededSyncrhonizationObjects()) return 1;

    VkShaderModule vertexShader;
    char* vertexShaderSrc = 
    "#version 450\n"
    "layout(location = 0) in vec2 inPosition;\n"
    "vec3 colors[3] = vec3[](vec3(1.0, 0.0, 0.0),vec3(0.0, 1.0, 0.0),vec3(0.0, 0.0, 1.0));"
    "layout(location = 0) out vec3 color;\n"
    "void main() {\n"
    "   gl_Position = vec4(inPosition,0.0f,1.0f); \n"
    "   color = colors[gl_VertexIndex]; \n"
    "}";

    if(!compileShader(vertexShaderSrc, shaderc_vertex_shader,&vertexShader)) return 1;

    VkShaderModule fragmentShader;
    char* fragmentShaderSrc = 
    "#version 450\n"
    "layout(location = 0) out vec4 outColor;\n"
    "layout(location = 0) in vec3 inColor;\n"
    "void main() {\n"
    "   outColor = vec4(inColor,1.0f);"
    "\n}";
    if(!compileShader(fragmentShaderSrc, shaderc_fragment_shader,&fragmentShader)) return 1;

    if(!initGraphicsPipeline(vertexShader,fragmentShader)) return 1;
    
    uint32_t indices[] = {
        0,2,1
    };

    Vertex vertices[] = {
        {-0.5f,0.5f},
        {0.5f,0.5f},
        {0.0f,-0.5f},
    };

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;

    VkBufferCreateInfo vertexBufferCreateInfo = {0};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.pNext = NULL;
    vertexBufferCreateInfo.flags = 0;
    vertexBufferCreateInfo.size = sizeof(vertices);
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertexBufferCreateInfo.queueFamilyIndexCount = 0;
    vertexBufferCreateInfo.pQueueFamilyIndices = NULL;
    vkCreateBuffer(device, &vertexBufferCreateInfo, NULL, &vertexBuffer);

    VkMemoryRequirements vertexMemRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &vertexMemRequirements);
    VkMemoryAllocateInfo vertexMemoryAllocateInfo = {0};
    vertexMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vertexMemoryAllocateInfo.pNext = NULL;
    vertexMemoryAllocateInfo.allocationSize = vertexMemRequirements.size;
    vertexMemoryAllocateInfo.memoryTypeIndex = findMemoryType(vertexMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(device, &vertexMemoryAllocateInfo,NULL,&vertexMemory);

    vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0);

    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;

    VkBufferCreateInfo indexBufferCreateInfo = {0};
    indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferCreateInfo.pNext = NULL;
    indexBufferCreateInfo.flags = 0;
    indexBufferCreateInfo.size = sizeof(indices);
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    indexBufferCreateInfo.queueFamilyIndexCount = 0;
    indexBufferCreateInfo.pQueueFamilyIndices = NULL;
    vkCreateBuffer(device, &indexBufferCreateInfo, NULL, &indexBuffer);

    VkMemoryRequirements indexMemRequirements;
    vkGetBufferMemoryRequirements(device, indexBuffer, &indexMemRequirements);
    VkMemoryAllocateInfo indexMemoryAllocateInfo = {0};
    indexMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    indexMemoryAllocateInfo.pNext = NULL;
    indexMemoryAllocateInfo.allocationSize = indexMemRequirements.size;
    indexMemoryAllocateInfo.memoryTypeIndex = findMemoryType(indexMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(device, &indexMemoryAllocateInfo,NULL,&indexMemory);

    vkBindBufferMemory(device, indexBuffer, indexMemory, 0);

    void *mapped;
    vkMapMemory(device, vertexMemory, 0, sizeof(vertices), 0, &mapped);
    memcpy(mapped,vertices,sizeof(vertices));
    vkUnmapMemory(device, vertexMemory);

    vkMapMemory(device, indexMemory, 0, sizeof(indices), 0, &mapped);
    memcpy(mapped,indices,sizeof(indices));
    vkUnmapMemory(device, indexMemory);

    uint32_t imageIndex;

    while(platform_still_running()){
        platform_window_handle_events();

        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence);

        vkResetCommandBuffer(cmd, 0);

        vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,imageAvailableSemaphore,NULL, &imageIndex);
    
        VkCommandBufferBeginInfo commandBufferBeginInfo = {0};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = NULL;
    
        vkBeginCommandBuffer(cmd,&commandBufferBeginInfo);

        VkRenderPassBeginInfo renderPassBeginInfo = {0};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers.items[imageIndex];
        VkOffset2D offset = {0,0};
        renderPassBeginInfo.renderArea.offset = offset;
        renderPassBeginInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearValue = {0};
        clearValue.color.float32[0] = 0.0f;
        clearValue.color.float32[1] = 0.0f;
        clearValue.color.float32[2] = 0.0f;
        clearValue.color.float32[3] = 1.0f;

        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        VkViewport viewport = {0};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = swapchainExtent.width;
        viewport.height = swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        
        VkRect2D scissor = {0};
        scissor.offset = offset;
        scissor.extent = swapchainExtent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize vOffset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &vOffset);
        vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd,ARRAY_LEN(indices),1,0,0,0);
    
        vkCmdEndRenderPass(cmd);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo = {0};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);

        VkPresentInfoKHR presentInfo = {0};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = NULL; // Optional
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    return 0;
}