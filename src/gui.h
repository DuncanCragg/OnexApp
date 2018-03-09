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
#include <items.h>
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
  void drawObjectProperties(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
  void drawNewPropertyValueEditor(char* path, char* val, bool single, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
  void drawPadding(char* path, int16_t width, int16_t height, int8_t depth);
  void drawNewValueOrObjectButton(char* path, int16_t width, int j, int8_t depth, bool valueToo);
  object* createNewObjectLikeOthers(char* path);
  object* createNewObjectForPropertyName(char* path, char* name);
  object* createNewEvent(struct tm* thisdate);
  int16_t calculateScrollerHeight(char* path, int16_t height);
  void getSummary(char* path, char* summary);
  bool getSummaryFrom(char* path, char* summary, const char* key);
  int16_t calculateKeyWidth(char* path);
  void drawObjectHeader(char* path, bool locallyEditable, int16_t width, int8_t depth);
  void drawObjectFooter(char* path, bool locallyEditable, int16_t width, int16_t keyWidth, int8_t depth);
  void drawNestedObjectPropertiesList(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
  void drawKey(char* path, char* key, int16_t width, int16_t height, int16_t keyWidth, int8_t depth);
  void drawPropertyList(char* pathkey, char* key, bool locallyEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth);
  void drawCalendar(char* path, int16_t width, int16_t height);
  void drawDayCell(char* path, struct tm* thisdate, int day, int col, int16_t width);
  void getCellTitles(char* titles, struct tm* thisdate, int col);
  void getCellEventsAndShowOpen(struct tm* thisdate, int col);
  void saveDays(char* path);
  void saveDay(char* path, int j, int col);
  void makeLink();
  void drawLink();
  void trackLink(bool from, char* path, int width);
  char* getLastLink();
  void setPropertyName(char* path , char* name);
  void setPropertyNameAndObject(char* path , char* name);
  void setPropertyNameAndLink(char* path , char* name);
  void setNewValue(char* path, char* valBuf, bool single);
  void hideKeyboard();
  void showKeyboard(float multy);
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

