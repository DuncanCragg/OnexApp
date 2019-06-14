/*
* GUI using Vulkan for OnexApp
*
* Based on Sascha Willems' imGui Vulkan Example
*
* Those parts Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <time.h>
#include <imgui.h>
#include "gui.h"
#include "im-gui.h"
#include "calendar.h"

GUI::GUI(VulkanBase* a, object* u, object* c)
{
  static_gui = this;
  app = a;
  user = u;
  config = c;
};

void GUI::changed()
{
  set_time_save_days();
}

void GUI::prepare()
{
  device = app->vulkanDevice;
  init_imgui((float)app->width, (float)app->height);
  get_font_info();
  createFontImage();
  setUpKeyMap();
  setupImageBuffer(app->queue);
  createSampler();
  setupDescriptorPool();
  setupDescriptorSetLayout();
  setupDescriptorSets();
  createPipelineCache();
  createPipelines(app->renderPass);
}

// ---------------------------------------------------------------------------------------------

static int framecount=0;
static int framewhendown=0;
#define KEY_UP_FRAME_DELAY 2
static uint32_t pendingKeyCodeUp=0;

void GUI::render()
{
  set_scaling();

  framecount++;
  ImGuiIO& io = ImGui::GetIO();

  if(framecount>=framewhendown+KEY_UP_FRAME_DELAY && pendingKeyCodeUp){
    keyReleased(pendingKeyCodeUp);
    pendingKeyCodeUp=0;
  }

  io.DisplaySize = ImVec2((float)app->width, (float)app->height);
  io.DeltaTime = app->frameTimer;
  io.FontGlobalScale = ((float)app->height)/1500.0;

#if not defined(__ANDROID__)
  io.MousePos = ImVec2(app->mousePos.x, app->mousePos.y);
  io.MouseDown[0] = app->mouseButtons.left;
  io.MouseDown[1] = app->mouseButtons.right;
  io.MouseDown[2] = app->mouseButtons.middle;
  io.MouseWheel   = app->mouseButtons.wheelUp? -0.2f : app->mouseButtons.wheelDown ? 0.2f : 0.0f;
  app->mouseButtons.wheelUp = false;
  app->mouseButtons.wheelDown = false;
#else
  io.MousePos = ImVec2(app->touchPos.x, app->touchPos.y);
  io.MouseDown[0] = app->touchDown;
  io.MouseDown[1] = false;
  io.MouseDown[2] = false;
  io.MouseWheel   = 0.0f;
#endif
  addAnyKeySym();
}

void GUI::setUpKeyMap()
{
  ImGuiIO& io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = KEY_LEFT_ARROW;
  io.KeyMap[ImGuiKey_RightArrow] = KEY_RIGHT_ARROW;
  io.KeyMap[ImGuiKey_UpArrow] = KEY_UP_ARROW;
  io.KeyMap[ImGuiKey_DownArrow] = KEY_DOWN_ARROW;
  io.KeyMap[ImGuiKey_Home] = KEY_HOME;
  io.KeyMap[ImGuiKey_End] = KEY_END;
  io.KeyMap[ImGuiKey_Delete] = KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = KEY_A;
  io.KeyMap[ImGuiKey_C] = KEY_C;
  io.KeyMap[ImGuiKey_V] = KEY_V;
  io.KeyMap[ImGuiKey_X] = KEY_X;
  io.KeyMap[ImGuiKey_Y] = KEY_Y;
  io.KeyMap[ImGuiKey_Z] = KEY_Z;
#if defined(__ANDROID__)
  io.KeyRepeatDelay = 1e20;
#endif
}

char32_t u32keyToAdd = 0;

void GUI::addAnyKeySym()
{
  if(u32keyToAdd){
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(u32keyToAdd);
#ifdef LOG_UNICODE_BYTES
    uint8_t* u32b = (uint8_t*)&u32keyToAdd; log_write("bytes: %x %x %x %x\n", u32b[3], u32b[2], u32b[1], u32b[0]);
#endif
    u32keyToAdd = 0;
  }
}

void GUI::keyPressed(uint32_t keyCode, char32_t u32key)
{
  framewhendown=framecount;
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  if(keyCode==BACK_BUTTON){ keyboardCancelled=true; return; }
#endif
  ImGuiIO& io = ImGui::GetIO();
  if(keyCode) io.KeysDown[keyCode] = true;
  io.KeyCtrl = io.KeysDown[KEY_CTRL_LEFT] || io.KeysDown[KEY_CTRL_RIGHT];
  io.KeyShift = io.KeysDown[KEY_SHIFT_LEFT] || io.KeysDown[KEY_SHIFT_RIGHT];
  io.KeyAlt = io.KeysDown[KEY_ALT_LEFT] || io.KeysDown[KEY_ALT_RIGHT];
  io.KeySuper = io.KeysDown[KEY_SUPER_LEFT] || io.KeysDown[KEY_SUPER_RIGHT];
  if(u32key) u32keyToAdd = u32key;
}

void GUI::keyReleased(uint32_t keyCode)
{
  if(framecount<framewhendown+KEY_UP_FRAME_DELAY){ pendingKeyCodeUp=keyCode; return; }
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  if(keyCode==BACK_BUTTON){  keyboardCancelled=false; return; }
#endif
  ImGuiIO& io = ImGui::GetIO();
  if(keyCode) io.KeysDown[keyCode] = false;
  io.KeyCtrl = io.KeysDown[KEY_CTRL_LEFT] || io.KeysDown[KEY_CTRL_RIGHT];
  io.KeyShift = io.KeysDown[KEY_SHIFT_LEFT] || io.KeysDown[KEY_SHIFT_RIGHT];
  io.KeyAlt = io.KeysDown[KEY_ALT_LEFT] || io.KeysDown[KEY_ALT_RIGHT];
  io.KeySuper = io.KeysDown[KEY_SUPER_LEFT] || io.KeysDown[KEY_SUPER_RIGHT];
}

// ---------------------------------------------------------------------------------------------

void GUI::createFontImage()
{
  VkImageCreateInfo imageInfo = vks::initializers::imageCreateInfo();
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo.extent.width = texWidth;
  imageInfo.extent.height = texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageInfo, nullptr, &fontImage));

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(device->logicalDevice, fontImage, &memReqs);
  VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &fontMemory));
  VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, fontImage, fontMemory, 0));

  VkImageViewCreateInfo viewInfo = vks::initializers::imageViewCreateInfo();
  viewInfo.image = fontImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = 1;
  VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &fontView));
}

void GUI::setupImageBuffer(VkQueue copyQueue)
{
  vks::Buffer stagingBuffer;

  VkDeviceSize uploadSize = texWidth*texHeight * 4 * sizeof(char);

  VK_CHECK_RESULT(device->createBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &stagingBuffer,
    uploadSize));

  stagingBuffer.map();
  memcpy(stagingBuffer.mapped, fontData, uploadSize);
  stagingBuffer.unmap();

  VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

  vks::tools::setImageLayout(
    copyCmd,
    fontImage,
    VK_IMAGE_ASPECT_COLOR_BIT,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_HOST_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT);

  VkBufferImageCopy bufferCopyRegion = {};
  bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  bufferCopyRegion.imageSubresource.layerCount = 1;
  bufferCopyRegion.imageExtent.width = texWidth;
  bufferCopyRegion.imageExtent.height = texHeight;
  bufferCopyRegion.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(
    copyCmd,
    stagingBuffer.buffer,
    fontImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &bufferCopyRegion
  );

  vks::tools::setImageLayout(
    copyCmd,
    fontImage,
    VK_IMAGE_ASPECT_COLOR_BIT,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  device->flushCommandBuffer(copyCmd, copyQueue, true);

  stagingBuffer.destroy();
}

void GUI::createSampler()
{
  VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler));
}

void GUI::setupDescriptorPool()
{
  std::vector<VkDescriptorPoolSize> poolSizes = {
    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
  };
  VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
  VK_CHECK_RESULT(vkCreateDescriptorPool(device->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void GUI::setupDescriptorSetLayout()
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
    vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
  };

  VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);

  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));

  VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

  VK_CHECK_RESULT(vkCreatePipelineLayout(device->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void GUI::setupDescriptorSets()
{
  VkDescriptorImageInfo fontDescriptor = vks::initializers::descriptorImageInfo(sampler, fontView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

  VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &allocInfo, &descriptorSet));

  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
    vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
  };
  vkUpdateDescriptorSets(device->logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void GUI::createPipelineCache()
{
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  VK_CHECK_RESULT(vkCreatePipelineCache(device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

void GUI::createPipelines(VkRenderPass renderPass)
{
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
    vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  VkPipelineRasterizationStateCreateInfo rasterizationState =
    vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.blendEnable = VK_TRUE;
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlendState =
    vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

  VkPipelineDepthStencilStateCreateInfo depthStencilState =
    vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

  VkPipelineViewportStateCreateInfo viewportState =
    vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

  VkPipelineMultisampleStateCreateInfo multisampleState =
    vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

  std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamicState =
    vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;
  pipelineCreateInfo.stageCount = shaderStages.size();
  pipelineCreateInfo.pStages = shaderStages.data();

  std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
    vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
  };
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
    vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),
    vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),
    vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),
  };
  VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
  vertexInputState.vertexBindingDescriptionCount = vertexInputBindings.size();
  vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
  vertexInputState.vertexAttributeDescriptionCount = vertexInputAttributes.size();
  vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

  pipelineCreateInfo.pVertexInputState = &vertexInputState;
  shaderStages[0] = app->loadShader(app->getAssetPath() + "shaders/imgui/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  shaderStages[1] = app->loadShader(app->getAssetPath() + "shaders/imgui/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(device->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

void GUI::updateBuffers()
{
  ImDrawData* imDrawData = ImGui::GetDrawData();

  // Note: Alignment is done inside buffer creation
  VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  // Update buffers only if vertex or index count has been changed compared to current buffer size

  if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
    vertexBuffer.unmap();
    vertexBuffer.destroy();
    VK_CHECK_RESULT(device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer, vertexBufferSize));
    vertexCount = imDrawData->TotalVtxCount;
    vertexBuffer.unmap();
    vertexBuffer.map();
  }

  VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
  if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
    indexBuffer.unmap();
    indexBuffer.destroy();
    VK_CHECK_RESULT(device->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &indexBuffer, indexBufferSize));
    indexCount = imDrawData->TotalIdxCount;
    indexBuffer.map();
  }

  ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
  ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }

  vertexBuffer.flush();
  indexBuffer.flush();
}

void GUI::drawFrame(VkCommandBuffer commandBuffer)
{
  ImGuiIO& io = ImGui::GetIO();

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

  VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
  pushConstBlock.translate = glm::vec2(-1.0f);
  vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

  ImDrawData* imDrawData = ImGui::GetDrawData();
  int32_t vertexOffset = 0;
  int32_t indexOffset = 0;
  for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
  {
    const ImDrawList* cmd_list = imDrawData->CmdLists[i];
    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
    {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
      VkRect2D scissorRect;
      scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
      scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
      scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
      scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
      vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
      vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
      indexOffset += pcmd->ElemCount;
    }
    vertexOffset += cmd_list->VtxBuffer.Size;
  }
}

void GUI::buildCommandBuffers(int32_t i)
{
  if(i==0){
    draw_gui();
    updateBuffers();
  }
  drawFrame(app->drawCmdBuffers[i]);
}

GUI::~GUI()
{
  vertexBuffer.destroy();
  indexBuffer.destroy();
  vkDestroyImage(device->logicalDevice, fontImage, nullptr);
  vkDestroyImageView(device->logicalDevice, fontView, nullptr);
  vkFreeMemory(device->logicalDevice, fontMemory, nullptr);
  vkDestroySampler(device->logicalDevice, sampler, nullptr);
  vkDestroyPipelineCache(device->logicalDevice, pipelineCache, nullptr);
  vkDestroyPipeline(device->logicalDevice, pipeline, nullptr);
  vkDestroyPipelineLayout(device->logicalDevice, pipelineLayout, nullptr);
  vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayout, nullptr);
}
