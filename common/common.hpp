#pragma once

#include <stdio.h>
#include <vulkan/vulkan.h>

void *cocoa_windowCreate(int width, int height, const char *title);
void *cocoa_windowGetLayer(void *cocoaWindow);

void VULKAN_CHECK(VkResult result);

struct Context {
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice gpu = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue queue = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkShaderModule shaderModules[2];
  VkPipelineShaderStageCreateInfo shaderStages[2];
  VkPipelineColorBlendAttachmentState colourBlendAttachmentState;
  VkPipelineColorBlendStateCreateInfo blend;
  VkPipelineDepthStencilStateCreateInfo depthStencil;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo raster;
  VkPipelineViewportStateCreateInfo viewport;
  VkPipelineMultisampleStateCreateInfo multisample;
  VkPipelineDynamicStateCreateInfo dynamic;
  VkGraphicsPipelineCreateInfo basePipelineCreateInfo;
  void *cocoaWindow = nullptr;
  int32_t queueFamilyIndex = -1;
};

void initializeContext(Context &context, const char *windowName);
void destroyContext(Context &context);

void createPipeline(Context &context);
void destroyPipeline(Context &context);
