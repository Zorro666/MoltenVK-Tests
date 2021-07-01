#include "common/common.hpp"

int main(int argc, const char * argv[])
{
  Context context;
  initializeContext(context, "vk_parameter_zoo");

  /*
   when rasterization is disabled, a lot of state should be ignored
   */
  context.rasterStateCreateInfo.rasterizerDiscardEnable = VK_TRUE;
  context.basePipelineCreateInfo.pViewportState = (VkPipelineViewportStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pMultisampleState = (VkPipelineMultisampleStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pDepthStencilState = (VkPipelineDepthStencilStateCreateInfo*)0x1234;
  context.basePipelineCreateInfo.pColorBlendState = (VkPipelineColorBlendStateCreateInfo*)0x1234;
  createPipeline(context);
  destroyPipeline(context);

  printf("Bug2: rasterizerDiscardEnable TRUE set various rasterisation pointers to 0x1234\n");

  destroyContext(context);
  return 1;
}
