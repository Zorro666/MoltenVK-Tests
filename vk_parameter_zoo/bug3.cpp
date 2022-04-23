#include <algorithm>

#include "common/common.h"

int main(int argc, const char * argv[])
{
  Context context(argc, argv);
  initializeContext(context, "vk_parameter_zoo");

  /*
   updates to a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER descriptor with immutable samplers does not modify the samplers (the image views are updated, but the sampler updates are ignored).
   
   VkDescriptorImageInfo

   sampler is a sampler handle, and is used in descriptor updates for types VK_DESCRIPTOR_TYPE_SAMPLER and VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER if the binding being updated does not use immutable samplers.
   
   If the descriptor type is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, the sampler member of the pImageInfo parameter is ignored and the immutable sampler is taken from the push descriptor set layout in the pipeline layout.
   
   If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and dstSet was not allocated with a layout that included immutable samplers for dstBinding with descriptorType, the sampler member of each element of pImageInfo must be a valid VkSampler object
   */

  VkSampler validSampler;
  VkSamplerCreateInfo samplerCreateInfo;
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.anisotropyEnable = VK_FALSE;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  samplerCreateInfo.compareEnable = VK_FALSE;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerCreateInfo.flags = 0;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.maxAnisotropy = 0.0f;
  samplerCreateInfo.maxLod = 0.0f;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minLod = 0.0f;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.pNext = nullptr;
  samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
  VULKAN_CHECK(vkCreateSampler(context.device, &samplerCreateInfo, NULL, &validSampler));
  VkSampler invalidSampler = (VkSampler)0x1234;

  VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2];

  descriptorSetLayoutBindings[0].binding = 0;
  descriptorSetLayoutBindings[0].descriptorCount = 1;
  descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorSetLayoutBindings[0].pImmutableSamplers = &validSampler;
  descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  descriptorSetLayoutBindings[1].binding = 99;
  descriptorSetLayoutBindings[1].descriptorCount = 1;
  descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorSetLayoutBindings[1].pImmutableSamplers = &invalidSampler;
  descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayout descriptorSetLayout;

  VkDescriptorSetLayoutCreateInfo layoutInfo;
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 2;
  layoutInfo.flags = 0;
  layoutInfo.pBindings = descriptorSetLayoutBindings;
  layoutInfo.pNext = nullptr;

  VULKAN_CHECK(vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &descriptorSetLayout));

  VkDescriptorPoolSize poolSizes[] =
  {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024},
  };

  VkDescriptorPoolCreateInfo descPoolCreateInfo;
  descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descPoolCreateInfo.flags = 0;
  descPoolCreateInfo.maxSets = 128;
  descPoolCreateInfo.pNext = nullptr;
  descPoolCreateInfo.pPoolSizes = poolSizes;
  descPoolCreateInfo.poolSizeCount = std::size(poolSizes);

  VkDescriptorPool descriptorPool;
  VULKAN_CHECK(vkCreateDescriptorPool(context.device, &descPoolCreateInfo, nullptr, &descriptorPool));
  
  VkDescriptorSetAllocateInfo descSetAllocateInfo;
  descSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descSetAllocateInfo.descriptorPool = descriptorPool;
  descSetAllocateInfo.descriptorSetCount = 1;
  descSetAllocateInfo.pNext = nullptr;
  descSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

  VkImageCreateInfo imageCreateInfo;
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.extent.height = 4;
  imageCreateInfo.extent.width = 4;
  imageCreateInfo.flags = 0;
  imageCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.pNext = nullptr;
  imageCreateInfo.pQueueFamilyIndices = nullptr;
  imageCreateInfo.queueFamilyIndexCount = 0;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
  VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  VkImage validImage;
  VULKAN_CHECK(vkCreateImage(context.device, &imageCreateInfo, nullptr, &validImage));
  
  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(context.device, validImage, &memoryRequirements);

  VkMemoryAllocateInfo allocateInfo;
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocateInfo.allocationSize = memoryRequirements.size;
  allocateInfo.memoryTypeIndex = getMemoryTypeIndex(context, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  allocateInfo.pNext = nullptr;

  VkDeviceMemory imageMemory;
  VULKAN_CHECK(vkAllocateMemory(context.device, &allocateInfo, nullptr, &imageMemory));

  VULKAN_CHECK(vkBindImageMemory(context.device, validImage, imageMemory, 0));

  VkComponentMapping components;
  components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  
  VkImageSubresourceRange subresourceRange;
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseArrayLayer = 0;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
  subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

  VkImageViewCreateInfo imageViewCreateInfo;
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.components = components;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  imageViewCreateInfo.image = validImage;
  imageViewCreateInfo.pNext = nullptr;
  imageViewCreateInfo.subresourceRange = subresourceRange;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

  VkImageView validImgView;
  VULKAN_CHECK(vkCreateImageView(context.device, &imageViewCreateInfo, nullptr, &validImgView));

  VkDescriptorSet descriptorSet;
  VULKAN_CHECK(vkAllocateDescriptorSets(context.device, &descSetAllocateInfo, &descriptorSet));
  
  VkImageView invalidImgView = (VkImageView)0x1234;

  VkDescriptorImageInfo combinedImageInfo;
  combinedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  combinedImageInfo.imageView = validImgView;
  combinedImageInfo.sampler = validSampler;

  VkDescriptorImageInfo soloImageInfo;
  soloImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  soloImageInfo.imageView = validImgView;
  soloImageInfo.sampler = VK_NULL_HANDLE;

  VkWriteDescriptorSet writeSets[2];
  writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSets[0].descriptorCount = 1;
  writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeSets[0].dstArrayElement = 0;
  writeSets[0].dstBinding = 0;
  writeSets[0].dstSet = descriptorSet;
  writeSets[0].pBufferInfo = nullptr;
  writeSets[0].pImageInfo = &combinedImageInfo;
  writeSets[0].pNext = nullptr;
  writeSets[0].pTexelBufferView = nullptr;

  writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSets[1].descriptorCount = 1;
  writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeSets[1].dstArrayElement = 0;
  writeSets[1].dstBinding = 0;
  writeSets[1].dstSet = descriptorSet;
  writeSets[1].pBufferInfo = nullptr;
  writeSets[1].pImageInfo = &soloImageInfo;
  writeSets[1].pNext = nullptr;
  writeSets[1].pTexelBufferView = nullptr;
  VkCopyDescriptorSet copySet;
  vkUpdateDescriptorSets(context.device, 2, writeSets, 0, &copySet);

  // updates to a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER descriptor
  // with immutable samplers does not modify the samplers
  // (the image views are updated, but the sampler updates are ignored).
  soloImageInfo.sampler = invalidSampler;
  vkUpdateDescriptorSets(context.device, 2, writeSets, 0, &copySet);

  printf("Hello, World!\n");

  destroyContext(context);
  return 1;
}
