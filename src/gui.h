#ifndef GUI_H
#define GUI_H

/*
* Vulkan Example - imGui (https://github.com/ocornut/imgui)
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void ImStrncpy(char* dst, const char* src, size_t count);
#include <assert.h>
#include <vector>
#include <array>
#include <algorithm>

extern "C" {
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onf.h>
#include <onr.h>
#include <items.h>
}

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "../sascha/VulkanBase.h"
#include "../sascha/VulkanDevice.hpp"
#include "../sascha/VulkanBuffer.hpp"

class GUI {
public:
  VkSampler sampler;
  vks::Buffer vertexBuffer;
  vks::Buffer indexBuffer;
  int32_t vertexCount = 0;
  int32_t indexCount = 0;
  VkDeviceMemory fontMemory = VK_NULL_HANDLE;
  VkImage fontImage = VK_NULL_HANDLE;
  VkImageView fontView = VK_NULL_HANDLE;
  VkPipelineCache pipelineCache;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  VkDescriptorPool descriptorPool;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSet descriptorSet;
  vks::VulkanDevice *device;

  struct PushConstBlock {
    glm::vec2 scale;
    glm::vec2 translate;
  } pushConstBlock;

  GUI(VulkanBase *app);

  void prepare();
  void setUpKeyMap();
  void createFontImage();
  void setupImageBuffer(VkQueue copyQueue);
  void createSampler();
  void setupDescriptorPool();
  void setupDescriptorSetLayout();
  void setupDescriptorSets();
  void createPipelineCache();
  void createPipelines(VkRenderPass renderPass);
  void updateBuffers();
  void drawFrame(VkCommandBuffer commandBuffer);
  void buildCommandBuffers(int32_t i);
  void updateUniformBuffers(){}
  void addAnyKeySym();
  void keyPressed(uint32_t keyCode, char32_t u32key);
  void keyReleased(uint32_t keyCode);
  void render();

  ~GUI();
};

#endif
