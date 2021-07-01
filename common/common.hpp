#pragma once

#include <stdio.h>
#include <vulkan/vulkan.h>

void *cocoa_windowCreate(int width, int height, const char *title);
void *cocoa_windowGetLayer(void *cocoaWindow);

void VULKAN_CHECK(VkResult result);

struct Context {
  VkInstance instance = (VkInstance)VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = (VkPhysicalDevice)VK_NULL_HANDLE;
  VkDevice device = (VkDevice)VK_NULL_HANDLE;
  VkQueue queue = (VkQueue)VK_NULL_HANDLE;
  VkSurfaceKHR surface = (VkSurfaceKHR)VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = (VkPipelineLayout)VK_NULL_HANDLE;
  VkRenderPass renderPass = (VkRenderPass)VK_NULL_HANDLE;
  VkPipeline pipeline = (VkPipeline)VK_NULL_HANDLE;
  VkShaderModule shaderModules[2];
  VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2];
  VkPipelineColorBlendAttachmentState colourBlendAttachmentState;
  VkPipelineColorBlendStateCreateInfo colourBlendStateCreateInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
  VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
  VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo;
  VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
  VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
  VkGraphicsPipelineCreateInfo basePipelineCreateInfo;
  VkViewport viewport;
  VkRect2D scissor;

  void *cocoaWindow = nullptr;
  int32_t queueFamilyIndex = -1;
};

void initializeContext(Context &context, const char *windowName);
void destroyContext(Context &context);

void createPipeline(Context &context);
void destroyPipeline(Context &context);
