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

extern void showOrHideSoftKeyboard(bool show);

class GUI {
public:
  ImGuiWindowFlags window_flags = 0;
  void drawGUI();
  void drawView();
  void drawObjectProperties(char* path, bool locallyEditable);
  void drawNestedObjectProperties(char* path, bool locallyEditable, int height);
  void drawNewPropertyValueEditor(char* path, char* key, char* val, bool locallyEditable, uint16_t width, uint16_t height);
  void drawNewValueOrObjectButtons(char* path, uint8_t width);
  void drawNewPropertyCombo(char* path);
  void drawObjectHeader(char* path);
  void drawNestedObjectPropertiesList(char* path, bool locallyEditable, int height);
  void drawPropertyValue(char* path, char* key, char* val, bool locallyEditable);
  void drawPropertyList(char* pathkey, char* key, bool locallyEditable);
  void setPropertyName(char* path , char* name);
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

  object* user;

  VulkanBase *app;

  struct PushConstBlock {
    glm::vec2 scale;
    glm::vec2 translate;
  } pushConstBlock;

  GUI(VulkanBase *app, object* user);

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

