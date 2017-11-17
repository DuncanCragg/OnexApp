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
#include <assert.h>
#include <vector>
#include <array>
#include <algorithm>

extern "C" {
#include <onex-kernel/log.h>
#include <onf.h>
}

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include <vulkan/vulkan.h>
#include "../sascha/VulkanBase.h"
#include "../sascha/VulkanDevice.hpp"
#include "../sascha/VulkanBuffer.hpp"

class GUI {
public:
  ImGuiWindowFlags window_flags = 0;
  void drawInitial();
  void drawObjectProperties(properties* p, bool locallyEditable);
  void drawProperty(char* key, char* val);
  void drawXXX();
  void drawFinal();
  void showOrHideSoftKeyboard(bool show);
  unsigned char* fontData;
  int texWidth, texHeight;
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
  VulkanBase *app;

  struct PushConstBlock {
    glm::vec2 scale;
    glm::vec2 translate;
  } pushConstBlock;

  GUI(VulkanBase *app);

  void prepare();

  void initImGUI(float width, float height);

  void setUpKeyMap();
  void getFontInfo();
  void createFontImage();
  void setupImageBuffer(VkQueue copyQueue);
  void createSampler();
  void setupDescriptorPool();
  void setupDescriptorSetLayout();
  void setupDescriptorSets();
  void createPipelineCache();
  void createPipelines(VkRenderPass renderPass);

  void drawProperties(properties* p);
  void drawGUI();

  void updateBuffers();

  void drawFrame(VkCommandBuffer commandBuffer);

  void buildCommandBuffers(int32_t i);

  void updateUniformBuffers(){}

  void addAnyKeySym();

  void keyPressed(uint32_t keyCode, char* u8KeySym);

  void keyPressed(uint32_t keyCode, uint32_t ucKeySym);

  void keyPressed(uint32_t keyCode);

  void keyReleased(uint32_t keyCode);

  void render();

  ~GUI();
};

