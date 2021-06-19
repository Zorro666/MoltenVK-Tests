#include <stdio.h>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

extern void *cocoa_windowCreate(int width, int height, const char *title);
extern void *cocoa_windowGetLayer(void *cocoaWindow);

struct Context
{
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice gpu = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue queue = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;
  void* cocoaWindow = nullptr;
  int32_t queueFamilyIndex = -1;
};

static const char* VkResultToString(VkResult result)
{
  switch (result)
  {
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_EVENT_SET: return "VK_EVENT_SET";
    case VK_EVENT_RESET: return "VK_EVENT_RESET";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
    default: return "UNKNOWN ERROR";
  }
}

#define VULKAN_CHECK(x) \
do \
{ \
  VkResult err = x; \
  if (err) \
  { \
    fprintf(stderr, "Detected Vulkan error: 0x%08X '%s'", err, VkResultToString(err)); \
    abort(); \
  } \
} while (0)

static void initVkInstance(Context& context)
{
  printf("Initializing vulkan instance.");

  const char* extensions[2] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME };

  VkApplicationInfo app;
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.applicationVersion = VK_MAKE_VERSION(1,0,0);
  app.engineVersion = VK_MAKE_VERSION(1,0,0);
  app.pNext = nullptr;
  app.pApplicationName = "vk_parameter_zoo";
  app.pEngineName = "MoltenVK-Tests";
  app.apiVersion = VK_MAKE_API_VERSION(0,1,0,0);

  VkInstanceCreateInfo instanceCreateInfo;
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.flags = 0;
  instanceCreateInfo.pNext = nullptr;
  instanceCreateInfo.pApplicationInfo = &app;
  instanceCreateInfo.enabledExtensionCount = 2;
  instanceCreateInfo.ppEnabledExtensionNames = extensions;
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.ppEnabledLayerNames = nullptr;

  VULKAN_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &context.instance));
}

static void initVkDevice(Context& context)
{
  printf("Initializing vulkan device.");

  uint32_t gpusCount = 0;
  VULKAN_CHECK(vkEnumeratePhysicalDevices(context.instance, &gpusCount, nullptr));

  if (gpusCount < 1)
  {
    fprintf(stderr,"No physical devices found.");
    abort();
  }

  std::vector<VkPhysicalDevice> gpus(gpusCount);
  VULKAN_CHECK(vkEnumeratePhysicalDevices(context.instance, &gpusCount, gpus.data()));
  context.gpu = gpus[0];

  uint32_t queuesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(context.gpu, &queuesCount, nullptr);

  if (queuesCount < 1)
  {
    fprintf(stderr,"No physical device queues found.");
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

  std::vector<const char*> extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.pEnabledFeatures = nullptr;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
  deviceCreateInfo.queueCreateInfoCount  = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.enabledExtensionCount = uint32_t(extensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

  VULKAN_CHECK(vkCreateDevice(context.gpu, &deviceCreateInfo, nullptr, &context.device));
  vkGetDeviceQueue(context.device, context.queueFamilyIndex, 0, &context.queue);
}

static void initRenderPass(Context &context)
{
  VkAttachmentDescription attachment;
  attachment.flags = 0;
  attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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

  VULKAN_CHECK(vkCreateRenderPass(context.device, &renderPassCreateInfo, nullptr, &context.renderPass));
}

static void initPipeline(Context &context)
{
  VkPipelineLayoutCreateInfo layoutCreateInfo;
  layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCreateInfo.flags = 0;
  layoutCreateInfo.pNext = nullptr;
  layoutCreateInfo.pPushConstantRanges = nullptr;
  layoutCreateInfo.pSetLayouts = nullptr;
  layoutCreateInfo.pushConstantRangeCount = 0l;
  layoutCreateInfo.setLayoutCount = 0;
  VULKAN_CHECK(vkCreatePipelineLayout(context.device, &layoutCreateInfo, nullptr, &context.pipelineLayout));

  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.flags = 0;
  vertexInputInfo.pNext = nullptr;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.vertexBindingDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyInfo.flags = 0;
  inputAssemblyInfo.pNext = nullptr;
  inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
  inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo raster;
  raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster.depthClampEnable = VK_FALSE;
  raster.depthBiasConstantFactor = 0.0f;
  raster.depthBiasEnable = VK_FALSE;
  raster.depthBiasSlopeFactor = 0.0f;
  raster.depthClampEnable = VK_FALSE;
  raster.flags = 0;
  raster.pNext = nullptr;
  raster.polygonMode = VK_POLYGON_MODE_FILL;
  raster.rasterizerDiscardEnable = VK_FALSE;

  raster.cullMode  = VK_CULL_MODE_BACK_BIT;
  raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
  raster.lineWidth = 1.0f;

  VkPipelineColorBlendAttachmentState colourBlendAttachmentState;
  colourBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
  colourBlendAttachmentState.blendEnable = VK_FALSE;
  colourBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colourBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colourBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;

  VkPipelineColorBlendStateCreateInfo blend;
  blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend.blendConstants[0] = 0.0f;
  blend.blendConstants[1] = 0.0f;
  blend.blendConstants[2] = 0.0f;
  blend.blendConstants[3] = 0.0f;
  blend.flags = 0;
  blend.logicOp = VK_LOGIC_OP_NO_OP;
  blend.logicOpEnable = VK_FALSE;
  blend.pNext = nullptr;
  blend.attachmentCount = 1;
  blend.pAttachments    = &colourBlendAttachmentState;

  VkPipelineViewportStateCreateInfo viewport;
  viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport.flags = 0;
  viewport.pNext = nullptr;
  viewport.pScissors = nullptr;
  viewport.pViewports = nullptr;
  viewport.viewportCount = 0;
  viewport.scissorCount  = 0;

  VkPipelineDepthStencilStateCreateInfo depthStencil;
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.back = VkStencilOpState();
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_EQUAL;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.flags = 0;
  depthStencil.front = VkStencilOpState();
  depthStencil.maxDepthBounds = 1.0f;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.pNext = nullptr;
  depthStencil.stencilTestEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisample;
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.alphaToCoverageEnable = VK_FALSE;
  multisample.alphaToOneEnable = VK_FALSE;
  multisample.flags = 0;
  multisample.minSampleShading = 0.0f;
  multisample.pNext = nullptr;
  multisample.pSampleMask = nullptr;
  multisample.sampleShadingEnable = VK_FALSE;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDynamicStateCreateInfo dynamic;
  dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic.flags = 0;
  dynamic.pNext = nullptr;
  dynamic.pDynamicStates    = nullptr;
  dynamic.dynamicStateCount = 0;

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

  shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = nullptr;//load_shader_module(context, "triangle.vert");
  shaderStages[0].pName  = "main";

  shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = nullptr;//load_shader_module(context, "triangle.frag");
  shaderStages[1].pName  = "main";

  VkGraphicsPipelineCreateInfo pipe;
  
  pipe.basePipelineHandle = VK_NULL_HANDLE;
  pipe.basePipelineIndex = 0;
  pipe.flags = 0;
  pipe.layout = context.pipelineLayout;
  pipe.pColorBlendState = &blend;
  pipe.pDepthStencilState = &depthStencil;
  pipe.pDynamicState = &dynamic;
  pipe.pInputAssemblyState = &inputAssemblyInfo;
  pipe.pMultisampleState = &multisample;
  pipe.pNext = nullptr;
  pipe.pRasterizationState = &raster;
  pipe.pStages = shaderStages.data();
  pipe.pTessellationState = nullptr;
  pipe.pVertexInputState = &vertexInputInfo;
  pipe.pViewportState = &viewport;
  pipe.renderPass = context.renderPass;
  pipe.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipe.stageCount = uint32_t(shaderStages.size());
  pipe.subpass = 0;

  VULKAN_CHECK(vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipe, nullptr, &context.pipeline));

  vkDestroyShaderModule(context.device, shaderStages[0].module, nullptr);
  vkDestroyShaderModule(context.device, shaderStages[1].module, nullptr);
}

static void createSurface(Context &context)
{
  void* layer = cocoa_windowGetLayer(context.cocoaWindow);

  VkMetalSurfaceCreateInfoEXT createInfo;
  createInfo.pLayer = layer;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
  
  VULKAN_CHECK(vkCreateMetalSurfaceEXT(context.instance, &createInfo, nullptr, &context.surface));
}

int main(int argc, const char * argv[])
{
  Context context;
  
  initVkInstance(context);

  context.cocoaWindow = cocoa_windowCreate(640, 480, "vk_parameter_zoo");

  createSurface(context);
  
  initVkDevice(context);
  initRenderPass(context);
  initPipeline(context);

  printf("Hello, World!\n");
  return 1;
}
