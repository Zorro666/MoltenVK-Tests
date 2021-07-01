#include "common/common.hpp"

int main(int argc, const char * argv[])
{
  Context context;
  initializeContext(context, "vk_parameter_zoo_bug_1");

  /*
   pTessellationState is ignored if the pipeline does not include a tessellation control shader stage and tessellation evaluation shader stage.
   */
  context.basePipelineCreateInfo.pTessellationState = (VkPipelineTessellationStateCreateInfo *)0x1234;
  createPipeline(context);
  destroyPipeline(context);

  printf("Bug1 pTessellationState = 0x1234\n");

  destroyContext(context);
  return 1;
}
