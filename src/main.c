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
#include "engine/vulkan_initPipelineLayout.h"
#include "engine/vulkan_compileShader.h"
#include "engine/vulkan_initGraphicsPipeline.h"
#include "engine/vulkan_synchro.h"
#include "engine/vulkan_buffer.h"
#include "engine/platform.h"

int main(){
    platform_create_window("TRIEX",640,480);

    if(!initialize_vulkan()) return 1;
    if(!createSurface()) return 1;
    if(!getDevice()) return 1;
    if(!initSwapchain()) return 1;
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
    if(!createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,sizeof(vertices),&vertexBuffer,&vertexMemory)) return 1;
    if(!transferDataToMemory(vertexMemory, vertices, 0, sizeof(vertices))) return 1;

    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
    if(!createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,sizeof(indices),&indexBuffer,&indexMemory)) return 1;
    if(!transferDataToMemory(indexMemory, indices, 0, sizeof(indices))) return 1;

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

        // Transition image layout from undefined to color attachment optimal
        VkImageMemoryBarrier barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = swapchainImages.items[imageIndex];
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &barrier
        );
        
        VkRenderingAttachmentInfo colorAttachment = {0};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapchainImageViews.items[imageIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color.float32[0] = 0.0f;
        colorAttachment.clearValue.color.float32[1] = 0.0f;
        colorAttachment.clearValue.color.float32[2] = 0.0f;
        colorAttachment.clearValue.color.float32[3] = 1.0f;

        VkRenderingInfo renderingInfo = {0};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        VkOffset2D offset = {0};
        renderingInfo.renderArea.offset = offset;
        renderingInfo.renderArea.extent = swapchainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = NULL;
        renderingInfo.pStencilAttachment = NULL;

        vkCmdBeginRendering(cmd, &renderingInfo);

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

        vkCmdEndRendering(cmd);

        // Transition image layout from color attachment optimal to present src
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &barrier
        );

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
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    return 0;
}