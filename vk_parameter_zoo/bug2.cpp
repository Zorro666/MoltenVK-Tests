#include <iterator>

#include "common/common.hpp"

int main(int argc, const char * argv[])
{
  Context context;
  initializeContext(context, "vk_parameter_zoo");

  /*
   when rasterization is disabled, a lot of state should be ignored
   */
  context.raster.rasterizerDiscardEnable = VK_TRUE;
  context.basePipelineCreateInfo.pViewportState = (VkPipelineViewportStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pMultisampleState = (VkPipelineMultisampleStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pDepthStencilState = (VkPipelineDepthStencilStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pColorBlendState = (VkPipelineColorBlendStateCreateInfo*)0x1234;

  createPipeline(context);
  destroyPipeline(context);

  printf("Hello, World!\n");

  destroyContext(context);
  return 1;
}
