#include <stdlib.h>
#include <fstream>
#include <vector>
#include "common.h"

Context::Context(int argc, const char *argv[])
{
  enableValidationLayers = false;
  for (int i = 0; i < argc; ++i)
  {
    if (strcmp(argv[i], "--debug") == 0)
    {
      enableValidationLayers = true;
    }
  }
}

static const char *VkResultToString(VkResult result) {
  switch (result) {
  case VK_NOT_READY:
    return "VK_NOT_READY";
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL";
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY";
  case VK_ERROR_FRAGMENTATION_EXT:
    return "VK_ERROR_FRAGMENTATION_EXT";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
  case VK_ERROR_VALIDATION_FAILED_EXT:
    return "VK_ERROR_VALIDATION_FAILED_EXT";
  case VK_ERROR_INVALID_SHADER_NV:
    return "VK_ERROR_INVALID_SHADER_NV";
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
  case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR";
  default:
    return "UNKNOWN ERROR";
  }
}

void VULKAN_CHECK(VkResult result) {
  if (result != VK_SUCCESS) {
    fprintf(stderr, "Detected Vulkan error: 0x%08X '%s'\n", result,
            VkResultToString(result));
    abort();
  }
}

static void createSurface(Context &context) {
  VkMetalSurfaceCreateInfoEXT createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
  createInfo.pLayer = cocoa_windowGetLayer(context.cocoaWindow);
  createInfo.flags = 0;
  createInfo.pNext = nullptr;

  VULKAN_CHECK(vkCreateMetalSurfaceEXT(context.instance, &createInfo, nullptr,
                                       &context.surface));
}

static VkShaderModule createShaderModule(Context &context, const char *path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    fprintf(stderr, "ERROR opening file '%s'\n", path);
    abort();
  }

  file.seekg(0, std::ios_base::end);
  size_t fileSizeInBytes = file.tellg();
  auto buffer = new uint32_t[fileSizeInBytes / 4];
  file.seekg(0, std::ios_base::beg);
  file.read((char *)buffer, fileSizeInBytes);
  file.close();

  VkShaderModuleCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.codeSize = fileSizeInBytes;
  createInfo.pCode = buffer;

  VkShaderModule shaderModule;
  VULKAN_CHECK(vkCreateShaderModule(context.device, &createInfo, nullptr,
                                    &shaderModule));

  return shaderModule;
}

static void createVkInstance(Context &context) {

  VULKAN_CHECK(volkInitialize());

  uint32_t countAllLayers = 0;
  VULKAN_CHECK(vkEnumerateInstanceLayerProperties(&countAllLayers, nullptr));
  std::vector<VkLayerProperties> availInstLayers(countAllLayers);
  VULKAN_CHECK(vkEnumerateInstanceLayerProperties(&countAllLayers, availInstLayers.data()));
  
  for (auto i = 0; i < countAllLayers; ++i)
    printf("'%s':%d\n", availInstLayers[i].layerName, availInstLayers[i].implementationVersion);
  
  printf("Initializing vulkan instance.\n");

  const char *extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_EXT_METAL_SURFACE_EXTENSION_NAME,
    "VK_KHR_get_physical_device_properties2",
  };

  VkApplicationInfo app;
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app.pNext = nullptr;
  app.pApplicationName = "vk_parameter_zoo";
  app.pEngineName = "MoltenVK-Tests";
  app.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  
  const char* layers[] = {
    "VK_LAYER_KHRONOS_validation"
    //"VK_LAYER_LUNARG_standard_validation"
  };

  VkInstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.flags = 0;
  instanceCreateInfo.pNext = nullptr;
  instanceCreateInfo.pApplicationInfo = &app;
  instanceCreateInfo.enabledExtensionCount = std::size(extensions);
  instanceCreateInfo.ppEnabledExtensionNames = extensions;
  instanceCreateInfo.enabledLayerCount = context.enableValidationLayers ? 1 : 0;
  instanceCreateInfo.ppEnabledLayerNames = context.enableValidationLayers ? layers : nullptr;

  VULKAN_CHECK(
      vkCreateInstance(&instanceCreateInfo, nullptr, &context.instance));

  volkLoadInstance(context.instance);
}

static void createVkDevice(Context &context) {
  printf("Initializing vulkan device.\n");

  uint32_t gpusCount = 0;
  VULKAN_CHECK(
      vkEnumeratePhysicalDevices(context.instance, &gpusCount, nullptr));

  if (gpusCount < 1) {
    fprintf(stderr, "No physical devices found.\n");
    abort();
  }

  std::vector<VkPhysicalDevice> physicalDevices(gpusCount);
  VULKAN_CHECK(
      vkEnumeratePhysicalDevices(context.instance, &gpusCount, physicalDevices.data()));
  context.physicalDevice = physicalDevices[0];

  uint32_t queuesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queuesCount, nullptr);

  if (queuesCount < 1) {
    fprintf(stderr, "No physical device queues found.\n");
    abort();
  }

  context.queueFamilyIndex = 0;

  float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo queueCreateInfo;
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.flags = 0;
  queueCreateInfo.pNext = nullptr;
  queueCreateInfo.queueFamilyIndex = context.queueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  const char * extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_portability_subset",
  };

  VkDeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.pEnabledFeatures = nullptr;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.enabledExtensionCount = std::size(extensions);
  deviceCreateInfo.ppEnabledExtensionNames = extensions;

  VULKAN_CHECK(
      vkCreateDevice(context.physicalDevice, &deviceCreateInfo, nullptr, &context.device));
  vkGetDeviceQueue(context.device, context.queueFamilyIndex, 0, &context.queue);
}

static void createRenderPass(Context &context) {
  VkAttachmentDescription attachment;
  attachment.flags = 0;
  attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colourAttachmentReference;
  colourAttachmentReference.attachment = 0;
  colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  
  VkSubpassDescription subpass;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pDepthStencilAttachment = nullptr;
  subpass.pInputAttachments = nullptr;
  subpass.pPreserveAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;

  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colourAttachmentReference;

  VkSubpassDependency dependency;
  dependency.dependencyFlags = 0;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.pNext = nullptr;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachment;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &dependency;

  VULKAN_CHECK(vkCreateRenderPass(context.device, &renderPassCreateInfo,
                                  nullptr, &context.renderPass));
}

static void createShaderModules(Context &context) {
  context.shaderModules[0] = createShaderModule(context, "common/vert.spv");
  context.shaderModules[1] = createShaderModule(context, "common/frag.spv");
}

static void destroyShaderModules(Context &context) {
  vkDestroyShaderModule(context.device, context.shaderModules[0], nullptr);
  vkDestroyShaderModule(context.device, context.shaderModules[1], nullptr);
}

static void initializeBasePipeline(Context &context) {
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.flags = 0;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0l;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  VULKAN_CHECK(vkCreatePipelineLayout(context.device, &pipelineLayoutCreateInfo,
                                      nullptr, &context.pipelineLayout));

  context.vertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  context.vertexInputStateCreateInfo.flags = 0;
  context.vertexInputStateCreateInfo.pNext = nullptr;
  context.vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
  context.vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
  context.vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
  context.vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;

  context.inputAssemblyStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  context.inputAssemblyStateCreateInfo.flags = 0;
  context.inputAssemblyStateCreateInfo.pNext = nullptr;
  context.inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
  context.inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  context.rasterStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  context.rasterStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  context.rasterStateCreateInfo.depthBiasClamp = 0.0f;
  context.rasterStateCreateInfo.depthBiasConstantFactor = 0.0f;
  context.rasterStateCreateInfo.depthBiasEnable = VK_FALSE;
  context.rasterStateCreateInfo.depthBiasSlopeFactor = 0.0f;
  context.rasterStateCreateInfo.depthClampEnable = VK_FALSE;
  context.rasterStateCreateInfo.flags = 0;
  context.rasterStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  context.rasterStateCreateInfo.lineWidth = 1.0f;
  context.rasterStateCreateInfo.pNext = nullptr;
  context.rasterStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  context.rasterStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;

  context.colourBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  context.colourBlendAttachmentState.blendEnable = VK_FALSE;
  context.colourBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  context.colourBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  context.colourBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  context.colourBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  context.colourBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  context.colourBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;

  context.colourBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  context.colourBlendStateCreateInfo.blendConstants[0] = 0.0f;
  context.colourBlendStateCreateInfo.blendConstants[1] = 0.0f;
  context.colourBlendStateCreateInfo.blendConstants[2] = 0.0f;
  context.colourBlendStateCreateInfo.blendConstants[3] = 0.0f;
  context.colourBlendStateCreateInfo.flags = 0;
  context.colourBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
  context.colourBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  context.colourBlendStateCreateInfo.pNext = nullptr;
  context.colourBlendStateCreateInfo.attachmentCount = 1;
  context.colourBlendStateCreateInfo.pAttachments = &context.colourBlendAttachmentState;

  context.viewport.height = 480.0f;
  context.viewport.maxDepth = 1.0f;
  context.viewport.minDepth = 0.0f;
  context.viewport.width = 640.0f;
  context.viewport.x = 0.0f;
  context.viewport.y = 0.0f;
  
  context.scissor.extent.height = 480;
  context.scissor.extent.width = 640;
  context.scissor.offset.x = 0;
  context.scissor.offset.y = 0;

  VkPipelineViewportStateCreateInfo &viewportStateCreateInfo = context.viewportStateCreateInfo;
  viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.flags = 0;
  viewportStateCreateInfo.pNext = nullptr;
  viewportStateCreateInfo.pScissors = &context.scissor;
  viewportStateCreateInfo.pViewports = &context.viewport;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.scissorCount = 1;

  VkPipelineDepthStencilStateCreateInfo &depthStencilStateCreateInfo = context.depthStencilStateCreateInfo;
  depthStencilStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilStateCreateInfo.back = VkStencilOpState();
  depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_EQUAL;
  depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
  depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
  depthStencilStateCreateInfo.flags = 0;
  depthStencilStateCreateInfo.front = VkStencilOpState();
  depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
  depthStencilStateCreateInfo.minDepthBounds = 0.0f;
  depthStencilStateCreateInfo.pNext = nullptr;
  depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo &multisampleStateCreateInfo = context.multisampleStateCreateInfo;
  multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
  multisampleStateCreateInfo.flags = 0;
  multisampleStateCreateInfo.minSampleShading = 0.0f;
  multisampleStateCreateInfo.pNext = nullptr;
  multisampleStateCreateInfo.pSampleMask = nullptr;
  multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDynamicStateCreateInfo &dynamicStateCreateInfo = context.dynamicStateCreateInfo;
  dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.flags = 0;
  dynamicStateCreateInfo.pNext = nullptr;
  dynamicStateCreateInfo.pDynamicStates = nullptr;
  dynamicStateCreateInfo.dynamicStateCount = 0;

  context.shaderStageCreateInfos[0].sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  context.shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  context.shaderStageCreateInfos[0].module = context.shaderModules[0];
  context.shaderStageCreateInfos[0].pName = "main";
  context.shaderStageCreateInfos[0].flags = 0;
  context.shaderStageCreateInfos[0].pNext = nullptr;
  context.shaderStageCreateInfos[0].pSpecializationInfo = nullptr;

  context.shaderStageCreateInfos[1].sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  context.shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  context.shaderStageCreateInfos[1].module = context.shaderModules[1];
  context.shaderStageCreateInfos[1].pName = "main";
  context.shaderStageCreateInfos[1].flags = 0;
  context.shaderStageCreateInfos[1].pNext = nullptr;
  context.shaderStageCreateInfos[1].pSpecializationInfo = nullptr;

  VkGraphicsPipelineCreateInfo &pipelineCreateInfo = context.basePipelineCreateInfo;

  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = 0;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.layout = context.pipelineLayout;
  pipelineCreateInfo.pColorBlendState = &context.colourBlendStateCreateInfo;
  pipelineCreateInfo.pDepthStencilState = &context.depthStencilStateCreateInfo;
  pipelineCreateInfo.pDynamicState = &context.dynamicStateCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &context.inputAssemblyStateCreateInfo;
  pipelineCreateInfo.pMultisampleState = &context.multisampleStateCreateInfo;
  pipelineCreateInfo.pNext = nullptr;
  pipelineCreateInfo.pRasterizationState = &context.rasterStateCreateInfo;
  pipelineCreateInfo.pStages = context.shaderStageCreateInfos;
  pipelineCreateInfo.pTessellationState = nullptr;
  pipelineCreateInfo.pVertexInputState = &context.vertexInputStateCreateInfo;
  pipelineCreateInfo.pViewportState = &context.viewportStateCreateInfo;
  pipelineCreateInfo.renderPass = context.renderPass;
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.subpass = 0;
}

void initializeContext(Context &context, const char *windowName) {
  context.cocoaWindow = cocoa_windowCreate(640, 480, windowName);

  createVkInstance(context);
  createSurface(context);
  createVkDevice(context);
  createShaderModules(context);
  createRenderPass(context);
  initializeBasePipeline(context);
}

uint32_t getMemoryTypeIndex(Context& context, uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memoryProperties);
  
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
          return i;
      }
  }
  fprintf(stderr, "ERROR failed to memory type %u\n", typeFilter);
  abort();
}

void destroyContext(Context &context)
{
  destroyShaderModules(context);
}

void createPipeline(Context &context) {
  VULKAN_CHECK(vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1,
                                         &context.basePipelineCreateInfo,
                                         nullptr, &context.pipeline));
}

void destroyPipeline(Context &context) {
  vkDestroyPipeline(context.device, context.pipeline, NULL);
}
