/*
* Vulkan Example base class
*
* Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "VulkanBase.h"

std::vector<const char*> VulkanBase::args;

VkResult VulkanBase::createInstance(bool enableValidation)
{
  this->settings.validation = enableValidation;

// Validation can also be forced via a define
#if defined(_VALIDATION)
  this->settings.validation = true;
#endif

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = name.c_str();
  appInfo.pEngineName = name.c_str();
  appInfo.apiVersion = VK_API_VERSION_1_0;

  std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

  // Enable surface extensions depending on os
#if defined(_WIN32)
  instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
  instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
  instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  if (instanceExtensions.size() > 0)
  {
    if (settings.validation)
    {
      instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  }
  if (settings.validation)
  {
    instanceCreateInfo.enabledLayerCount = vks::debug::validationLayerCount;
    instanceCreateInfo.ppEnabledLayerNames = vks::debug::validationLayerNames;
  }
  return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

std::string VulkanBase::getWindowTitle()
{
  std::string device(deviceProperties.deviceName);
  std::string windowTitle;
  windowTitle = title + " - " + device;
  windowTitle += " - " + std::to_string(frameCounter) + " fps";
  return windowTitle;
}

#if !(defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// iOS & macOS: VulkanBase::getAssetPath() implemented externally to allow access to Objective-C components
const std::string VulkanBase::getAssetPath()
{
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  return "";
#elif defined(VK_EXAMPLE_DATA_DIR)
  return VK_EXAMPLE_DATA_DIR;
#else
  return "./../data/";
#endif
}
#endif

bool VulkanBase::checkCommandBuffers()
{
  for (auto& cmdBuffer : drawCmdBuffers)
  {
    if (cmdBuffer == VK_NULL_HANDLE)
    {
      return false;
    }
  }
  return true;
}

void VulkanBase::createCommandBuffers()
{
  // Create one command buffer for each swap chain image and reuse for rendering
  drawCmdBuffers.resize(swapChain.imageCount);

  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
    vks::initializers::commandBufferAllocateInfo(
      cmdPool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      static_cast<uint32_t>(drawCmdBuffers.size()));

  VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}

void VulkanBase::destroyCommandBuffers()
{
  vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}

VkCommandBuffer VulkanBase::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
  VkCommandBuffer cmdBuffer;

  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
    vks::initializers::commandBufferAllocateInfo(
      cmdPool,
      level,
      1);

  VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &cmdBuffer));

  // If requested, also start the new command buffer
  if (begin)
  {
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
  }

  return cmdBuffer;
}

void VulkanBase::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
{
  if (commandBuffer == VK_NULL_HANDLE)
  {
    return;
  }

  VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
  VK_CHECK_RESULT(vkQueueWaitIdle(queue));

  if (free)
  {
    vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
  }
}

void VulkanBase::createPipelineCache()
{
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

void VulkanBase::prepare()
{
  if (vulkanDevice->enableDebugMarkers)
  {
    vks::debugmarker::setup(device);
  }
  createCommandPool();
  setupSwapChain();
  createCommandBuffers();
  setupDepthStencil();
  setupRenderPass();
  createPipelineCache();
  setupFrameBuffer();
}

VkPipelineShaderStageCreateInfo VulkanBase::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
  VkPipelineShaderStageCreateInfo shaderStage = {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
  shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
#endif
  shaderStage.pName = "main"; // todo : make param
  assert(shaderStage.module != VK_NULL_HANDLE);
  shaderModules.push_back(shaderStage.module);
  return shaderStage;
}

void VulkanBase::renderFrame()
{
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
      viewUpdated = false;
      viewChanged();
    }

    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = (float)tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
      viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
      timer += timerSpeed * frameTimer;
      if (timer > 1.0)
      {
        timer -= 1.0f;
      }
    }
    fpsTimer += (float)tDiff;
    if (fpsTimer > 1000.0f)
    {
#if defined(_WIN32)
      std::string windowTitle = getWindowTitle();
      SetWindowText(window, windowTitle.c_str());
#endif
      lastFPS = static_cast<uint32_t>(1.0f / frameTimer);
      fpsTimer = 0.0f;
      frameCounter = 0;
    }
}

void VulkanBase::renderLoop()
{
  destWidth = width;
  destHeight = height;
#if defined(_WIN32)
  MSG msg;
  bool quitMessageReceived = false;
  while (!quitMessageReceived) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        quitMessageReceived = true;
        break;
      }
    }
    renderFrame();
  }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  while (1)
  {
    int ident;
    int events;
    struct android_poll_source* source;
    bool destroy = false;

    while ((ident = ALooper_pollAll(focused ? 0 : 500, NULL, &events, (void**)&source)) >= 0)
    {
      if (source != NULL)
      {
        source->process(androidApp, source);
      }
      if (androidApp->destroyRequested != 0)
      {
        LOGD("Android app destroy requested");
        destroy = true;
        break;
      }
    }

    if(!focused){
      if(alarm){
        alarm=false;
        loop();
      }
      if(ident==ALOOPER_POLL_TIMEOUT) continue;
    }


    // App destruction requested
    // Exit loop, example will be destroyed in application main
    if (destroy)
    {
      break;
    }

    // Render frame
    if (prepared)
    {
      auto tStart = std::chrono::high_resolution_clock::now();
      render();
      frameCounter++;
      auto tEnd = std::chrono::high_resolution_clock::now();
      auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
      frameTimer = tDiff / 1000.0f;
      camera.update(frameTimer);
      // Convert to clamped timer value
      if (!paused)
      {
        timer += timerSpeed * frameTimer;
        if (timer > 1.0)
        {
          timer -= 1.0f;
        }
      }
      fpsTimer += (float)tDiff;
      if (fpsTimer > 1000.0f)
      {
        lastFPS = frameCounter;
        fpsTimer = 0.0f;
        frameCounter = 0;
      }

      bool updateView = false;

      // Check touch state (for movement)
      if (touchDown) {
        touchTimer += frameTimer;
      }
      if (touchTimer >= 1.0) {
        camera.keys.up = true;
        viewChanged();
      }

      // Check gamepad state
      const float deadZone = 0.0015f;
      // todo : check if gamepad is present
      // todo : time based and relative axis positions
      if (camera.type != Camera::CameraType::firstperson)
      {
        // Rotate
        if (std::abs(gamePadState.axisLeft.x) > deadZone)
        {
          rotation.y += gamePadState.axisLeft.x * 0.5f * rotationSpeed;
          camera.rotate(glm::vec3(0.0f, gamePadState.axisLeft.x * 0.5f, 0.0f));
          updateView = true;
        }
        if (std::abs(gamePadState.axisLeft.y) > deadZone)
        {
          rotation.x -= gamePadState.axisLeft.y * 0.5f * rotationSpeed;
          camera.rotate(glm::vec3(gamePadState.axisLeft.y * 0.5f, 0.0f, 0.0f));
          updateView = true;
        }
        // Zoom
        if (std::abs(gamePadState.axisRight.y) > deadZone)
        {
          zoom -= gamePadState.axisRight.y * 0.01f * zoomSpeed;
          updateView = true;
        }
        if (updateView)
        {
          viewChanged();
        }
      }
      else
      {
        updateView = camera.updatePad(gamePadState.axisLeft, gamePadState.axisRight, frameTimer);
        if (updateView)
        {
          viewChanged();
        }
      }
    }
  }
#elif defined(_DIRECT2DISPLAY)
  while (!quit)
  {
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
      viewUpdated = false;
      viewChanged();
    }
    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
      viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
      timer += timerSpeed * frameTimer;
      if (timer > 1.0)
      {
        timer -= 1.0f;
      }
    }
    fpsTimer += (float)tDiff;
    if (fpsTimer > 1000.0f)
    {
      lastFPS = frameCounter;
      fpsTimer = 0.0f;
      frameCounter = 0;
    }
  }
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  while (!quit)
  {
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
      viewUpdated = false;
      viewChanged();
    }

    while (wl_display_prepare_read(display) != 0)
      wl_display_dispatch_pending(display);
    wl_display_flush(display);
    wl_display_read_events(display);
    wl_display_dispatch_pending(display);

    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
      viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
      timer += timerSpeed * frameTimer;
      if (timer > 1.0)
      {
        timer -= 1.0f;
      }
    }
    fpsTimer += (float)tDiff;
    if (fpsTimer > 1000.0f)
    {
      std::string windowTitle = getWindowTitle();
      wl_shell_surface_set_title(shell_surface, windowTitle.c_str());
      lastFPS = frameCounter;
      fpsTimer = 0.0f;
      frameCounter = 0;
    }
  }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  xcb_flush(connection);
  while (!quit)
  {
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
      viewUpdated = false;
      viewChanged();
    }
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(connection)))
    {
      handleEvent(event);
      free(event);
    }
    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
      viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
      timer += timerSpeed * frameTimer;
      if (timer > 1.0)
      {
        timer -= 1.0f;
      }
    }
    fpsTimer += (float)tDiff;
    if (fpsTimer > 1000.0f)
    {
      std::string windowTitle = getWindowTitle();
      xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                          window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                          windowTitle.size(), windowTitle.c_str());
      lastFPS = frameCounter;
      fpsTimer = 0.0f;
      frameCounter = 0;
    }
  }
#endif
  // Flush device to make sure all resources can be freed
  vkDeviceWaitIdle(device);
}

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1

void VulkanBase::basePipelines(VkPipelineLayout& pipelineLayout,
                               size_t vertSize,
                               size_t instSize,
                               std::string vertShader,
                               std::string fragShader,
                               VkPipeline* pipeline,
                               VkPipeline* pipelinesBlending,
                               VkPipeline* pipelinesWireframe)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
      vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
      vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
      vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlendState =
      vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
      vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
      vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
      vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState =
      vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), dynamicStateEnables.size(), 0);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo( pipelineLayout, renderPass, 0);

    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();

    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    if(instSize){
       // Vertex input bindings
       // The instancing pipeline uses a vertex input state with two bindings
       bindingDescriptions = {
         // Binding point 0: Mesh vertex layout description at per-vertex rate
         vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertSize, VK_VERTEX_INPUT_RATE_VERTEX),
         // Binding point 1: Instanced data at per-instance rate
         vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, instSize, VK_VERTEX_INPUT_RATE_INSTANCE)
       };
    } else {
       bindingDescriptions = {
         vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertSize, VK_VERTEX_INPUT_RATE_VERTEX)
       };
    }

    if(instSize){
        // Vertex attribute bindings
        // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
        // instanced.vert:
        //  layout (location = 0) in vec3 inPos;      Per-Vertex
        //  ...
        //  layout (location = 4) in vec3 instancePos;  Per-Instance
        attributeDescriptions = {
          // Per-vertex attributees
          // These are advanced for each vertex fetched by the vertex shader
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),      // Location 0: Position
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),  // Location 1: Normal
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),  // Location 2: Texture coordinates
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),  // Location 3: Color
          // Per-Instance attributes
          // These are fetched for each instance rendered
          vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),  // Location 4: Position
          vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32B32_SFLOAT, 0),      // Location 5: Rotation
          vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SFLOAT,sizeof(float) * 6),    // Location 6: Scale
          vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 7, VK_FORMAT_R32_SINT, sizeof(float) * 7),    // Location 7: Texture array layer index
        };
    } else {
        attributeDescriptions = {
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),
          vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8)
        };
    }

    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

    pipelineCreateInfo.pVertexInputState = &inputState;

    shaderStages[0] = loadShader(getAssetPath() + "shaders/" + vertShader, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getAssetPath() + "shaders/" + fragShader, VK_SHADER_STAGE_FRAGMENT_BIT);

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, pipeline));

    if(pipelinesBlending) {

        rasterizationState.cullMode = VK_CULL_MODE_NONE;
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, pipelinesBlending));
    }

    if(pipelinesWireframe) {
        if (deviceFeatures.fillModeNonSolid) {
          rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
          blendAttachmentState.blendEnable = VK_FALSE;
          rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
          rasterizationState.lineWidth = 1.0f;
          VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, pipelinesWireframe));
        }
    }
}

void VulkanBase::getTexFormat()
{
  if (vulkanDevice->features.textureCompressionBC) {
    texFormatSuffix = "_bc3_unorm";
    texFormat = VK_FORMAT_BC3_UNORM_BLOCK;
  }
  else if (vulkanDevice->features.textureCompressionASTC_LDR) {
    texFormatSuffix = "_astc_8x8_unorm";
    texFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
  }
  else if (vulkanDevice->features.textureCompressionETC2) {
    texFormatSuffix = "_etc2_unorm";
    texFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
  }
  else {
    vks::tools::exitFatal("Device does not support any compressed texture format!", "Error");
  }
}

void VulkanBase::setupDescriptorPool(VkDescriptorPool* pDescriptorPool, size_t n)
{
  std::vector<VkDescriptorPoolSize> poolSizes =
  {
    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, n),
    vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, n),
  };

  VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), n);

  VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, pDescriptorPool));
}

void VulkanBase::reBuildCommandBuffers()
{
  if (!checkCommandBuffers())
  {
    destroyCommandBuffers();
    createCommandBuffers();
  }
  buildCommandBuffers();
}

void VulkanBase::prepareFrame()
{
  // Acquire the next image from the swap chain
  VkResult err = swapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer);
  // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
  if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)) {
    windowResize();
  }
  else {
    VK_CHECK_RESULT(err);
  }
}

void VulkanBase::submitFrame()
{
  VK_CHECK_RESULT(swapChain.queuePresent(queue, currentBuffer, semaphores.renderComplete));
  VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}

VulkanBase::VulkanBase(bool enableValidation)
{
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
  // Check for a valid asset path
  struct stat info;
  if (stat(getAssetPath().c_str(), &info) != 0)
  {
#if defined(_WIN32)
    std::string msg = "Could not locate asset path in \"" + getAssetPath() + "\" !";
    MessageBox(NULL, msg.c_str(), "Fatal error", MB_OK | MB_ICONERROR);
#else
    std::cerr << "Error: Could not find asset path in " << getAssetPath() << std::endl;
#endif
    exit(-1);
  }
#endif

  settings.validation = enableValidation;

  // Parse command line arguments
  for (size_t i = 0; i < args.size(); i++)
  {
    if (args[i] == std::string("-validation")) {
      settings.validation = true;
    }
    if (args[i] == std::string("-vsync")) {
      settings.vsync = true;
    }
    if (args[i] == std::string("-fullscreen")) {
      settings.fullscreen = true;
    }
    if ((args[i] == std::string("-w")) || (args[i] == std::string("-width"))) {
      char* endptr;
      uint32_t w = strtol(args[i + 1], &endptr, 10);
      if (endptr != args[i + 1]) { width = w; };
    }
    if ((args[i] == std::string("-h")) || (args[i] == std::string("-height"))) {
      char* endptr;
      uint32_t h = strtol(args[i + 1], &endptr, 10);
      if (endptr != args[i + 1]) { height = h; };
    }
  }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  // Vulkan library is loaded dynamically on Android
  bool libLoaded = vks::android::loadVulkanLibrary();
  assert(libLoaded);
#elif defined(_DIRECT2DISPLAY)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  initWaylandConnection();
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  initxcbConnection();
#endif

#if defined(_WIN32)
  // Enable console if validation is active
  // Debug message callback will output to it
  if (this->settings.validation)
  {
    setupConsole("Vulkan validation output");
  }
#endif
}

void VulkanBase::startup()
{
  initVulkan();
  initSwapchain();
}

void VulkanBase::cleanup()
{
  prepared = false;
  swapChain.cleanup();
  if (descriptorPool != VK_NULL_HANDLE)
  {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  }
  destroyCommandBuffers();
  vkDestroyRenderPass(device, renderPass, nullptr);
  for (uint32_t i = 0; i < frameBuffers.size(); i++)
  {
    vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
  }
/*
  for (auto& shaderModule : shaderModules)
  {
    vkDestroyShaderModule(device, shaderModule, nullptr);
  }
*/
  vkDestroyImageView(device, depthStencil.view, nullptr);
  vkDestroyImage(device, depthStencil.image, nullptr);
  vkFreeMemory(device, depthStencil.mem, nullptr);

  vkDestroyPipelineCache(device, pipelineCache, nullptr);

  vkDestroyCommandPool(device, cmdPool, nullptr);

  vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
  vkDestroySemaphore(device, semaphores.renderComplete, nullptr);

  delete vulkanDevice;

  if (settings.validation)
  {
    vks::debug::freeDebugCallback(instance);
  }

  vkDestroyInstance(instance, nullptr);
}

VulkanBase::~VulkanBase()
{
  cleanup();

#if defined(_DIRECT2DISPLAY)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  wl_shell_surface_destroy(shell_surface);
  wl_surface_destroy(surface);
  if (keyboard)
    wl_keyboard_destroy(keyboard);
  if (pointer)
    wl_pointer_destroy(pointer);
  wl_seat_destroy(seat);
  wl_shell_destroy(shell);
  wl_compositor_destroy(compositor);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  // todo : android cleanup (if required)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);
#endif
}

void VulkanBase::initVulkan()
{
  VkResult err;

  // Vulkan instance
  err = createInstance(settings.validation);
  if (err) {
    vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), "Fatal error");
  }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  vks::android::loadVulkanFunctions(instance);
#endif

  // If requested, we enable the default validation layers for debugging
  if (settings.validation)
  {
    // The report flags determine what type of messages for the layers will be displayed
    // For validating (debugging) an appplication the error and warning bits should suffice
    VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    // Additional flags include performance info, loader and layer debug messages, etc.
    vks::debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);
  }

  // Physical device
  uint32_t gpuCount = 0;
  // Get number of available physical devices
  VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
  assert(gpuCount > 0);
  // Enumerate devices
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
  if (err) {
    vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), "Fatal error");
  }

  // GPU selection

  // Select physical device to be used for the Vulkan example
  // Defaults to the first device unless specified by command line
  uint32_t selectedDevice = 0;

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
  // GPU selection via command line argument
  for (size_t i = 0; i < args.size(); i++)
  {
    // Select GPU
    if ((args[i] == std::string("-g")) || (args[i] == std::string("-gpu")))
    {
      char* endptr;
      uint32_t index = strtol(args[i + 1], &endptr, 10);
      if (endptr != args[i + 1])
      {
        if (index > gpuCount - 1)
        {
          std::cerr << "Selected device index " << index << " is out of range, reverting to device 0 (use -listgpus to show available Vulkan devices)" << std::endl;
        }
        else
        {
          std::cout << "Selected Vulkan device " << index << std::endl;
          selectedDevice = index;
        }
      };
      break;
    }
    // List available GPUs
    if (args[i] == std::string("-listgpus"))
    {
      uint32_t gpuCount = 0;
      VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
      if (gpuCount == 0)
      {
        std::cerr << "No Vulkan devices found!" << std::endl;
      }
      else
      {
        // Enumerate devices
        std::cout << "Available Vulkan devices" << std::endl;
        std::vector<VkPhysicalDevice> devices(gpuCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data()));
        for (uint32_t i = 0; i < gpuCount; i++) {
          VkPhysicalDeviceProperties deviceProperties;
          vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
          std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
          std::cout << " Type: " << vks::tools::physicalDeviceTypeString(deviceProperties.deviceType) << std::endl;
          std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3ff) << "." << (deviceProperties.apiVersion & 0xfff) << std::endl;
        }
      }
    }
  }
#endif

  physicalDevice = physicalDevices[selectedDevice];

  // Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

  // Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
  getEnabledFeatures();

  // Vulkan device creation
  // This is handled by a separate class that gets a logical device representation
  // and encapsulates functions related to a device
  vulkanDevice = new vks::VulkanDevice(physicalDevice);
  VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledExtensions);
  if (res != VK_SUCCESS) {
    vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), "Fatal error");
  }
  device = vulkanDevice->logicalDevice;

  // Get a graphics queue from the device
  vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

  // Find a suitable depth format
  VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
  assert(validDepthFormat);

  swapChain.connect(instance, physicalDevice, device);

  // Create synchronization objects
  VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
  // Create a semaphore used to synchronize image presentation
  // Ensures that the image is displayed before we start submitting new commands to the queu
  VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
  // Create a semaphore used to synchronize command submission
  // Ensures that the image is not presented until all commands have been sumbitted and executed
  VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));
  // Create a semaphore used to synchronize command submission

  // Set up submit info structure
  // Semaphores will stay the same during application lifetime
  // Command buffer submission info is set by each example
  submitInfo = vks::initializers::submitInfo();
  submitInfo.pWaitDstStageMask = &submitPipelineStages;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &semaphores.presentComplete;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &semaphores.renderComplete;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  // Get Android device name and manufacturer (to display along GPU name)
  androidProduct = "";
  char prop[PROP_VALUE_MAX+1];
  int len = __system_property_get("ro.product.manufacturer", prop);
  if (len > 0) {
    androidProduct += std::string(prop) + " ";
  };
  len = __system_property_get("ro.product.model", prop);
  if (len > 0) {
    androidProduct += std::string(prop);
  };
  LOGD("androidProduct = %s", androidProduct.c_str());
#endif
}

#if defined(_WIN32)
// Win32 : Sets up a console window and redirects standard output to it
void VulkanBase::setupConsole(std::string title)
{
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  FILE *stream;
  freopen_s(&stream, "CONOUT$", "w+", stdout);
  freopen_s(&stream, "CONOUT$", "w+", stderr);
  SetConsoleTitle(TEXT(title.c_str()));
}

HWND VulkanBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
  this->windowInstance = hinstance;

  WNDCLASSEX wndClass;

  wndClass.cbSize = sizeof(WNDCLASSEX);
  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = wndproc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = 0;
  wndClass.hInstance = hinstance;
  wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndClass.lpszMenuName = NULL;
  wndClass.lpszClassName = name.c_str();
  wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

  if (!RegisterClassEx(&wndClass))
  {
    std::cout << "Could not register window class!\n";
    fflush(stdout);
    exit(1);
  }

  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

  if (settings.fullscreen)
  {
    DEVMODE dmScreenSettings;
    memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmPelsWidth = screenWidth;
    dmScreenSettings.dmPelsHeight = screenHeight;
    dmScreenSettings.dmBitsPerPel = 32;
    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
    {
      if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
      {
        if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
        {
          settings.fullscreen = false;
        }
        else
        {
          return nullptr;
        }
      }
    }

  }

  DWORD dwExStyle;
  DWORD dwStyle;

  if (settings.fullscreen)
  {
    dwExStyle = WS_EX_APPWINDOW;
    dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
  }
  else
  {
    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
  }

  RECT windowRect;
  windowRect.left = 0L;
  windowRect.top = 0L;
  windowRect.right = settings.fullscreen ? (long)screenWidth : (long)width;
  windowRect.bottom = settings.fullscreen ? (long)screenHeight : (long)height;

  AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

  std::string windowTitle = getWindowTitle();
  window = CreateWindowEx(0,
    name.c_str(),
    windowTitle.c_str(),
    dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    0,
    0,
    windowRect.right - windowRect.left,
    windowRect.bottom - windowRect.top,
    NULL,
    NULL,
    hinstance,
    NULL);

  if (!settings.fullscreen)
  {
    // Center on screen
    uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
    uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
    SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  if (!window)
  {
    printf("Could not create window!\n");
    fflush(stdout);
    return nullptr;
    exit(1);
  }

  ShowWindow(window, SW_SHOW);
  SetForegroundWindow(window);
  SetFocus(window);

  return window;
}

void VulkanBase::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CLOSE:
    prepared = false;
    DestroyWindow(hWnd);
    PostQuitMessage(0);
    break;
  case WM_PAINT:
    ValidateRect(window, NULL);
    break;
  case WM_KEYDOWN:
    switch (wParam)
    {
    case KEY_P:
      paused = !paused;
      break;
    case KEY_ESCAPE:
      PostQuitMessage(0);
      break;
    }

    if (camera.firstperson)
    {
      switch (wParam)
      {
      case KEY_W:
        camera.keys.up = true;
        break;
      case KEY_S:
        camera.keys.down = true;
        break;
      case KEY_A:
        camera.keys.left = true;
        break;
      case KEY_D:
        camera.keys.right = true;
        break;
      }
    }

    keyPressed((uint32_t)wParam);
    break;
  case WM_KEYUP:
    if (camera.firstperson)
    {
      switch (wParam)
      {
      case KEY_W:
        camera.keys.up = false;
        break;
      case KEY_S:
        camera.keys.down = false;
        break;
      case KEY_A:
        camera.keys.left = false;
        break;
      case KEY_D:
        camera.keys.right = false;
        break;
      }
    }
    keyReleased((uint32_t)wParam);
    break;
  case WM_LBUTTONDOWN:
    mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
    mouseButtons.left = true;
    break;
  case WM_RBUTTONDOWN:
    mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
    mouseButtons.right = true;
    break;
  case WM_MBUTTONDOWN:
    mousePos = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
    mouseButtons.middle = true;
    break;
  case WM_LBUTTONUP:
    mouseButtons.left = false;
    break;
  case WM_RBUTTONUP:
    mouseButtons.right = false;
    break;
  case WM_MBUTTONUP:
    mouseButtons.middle = false;
    break;
  case WM_MOUSEWHEEL:
  {
    short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    zoom += (float)wheelDelta * 0.005f * zoomSpeed;
    camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed));
    viewUpdated = true;
    break;
  }
  case WM_MOUSEMOVE:
  {
    bool handled = false;
    int32_t posx = LOWORD(lParam);
    int32_t posy = HIWORD(lParam);
    mouseMoved((float)posx, (float)posy, handled);
    if (handled) {
      mousePos = glm::vec2((float)posx, (float)posy);
      break;
    }
    if (mouseButtons.right) {
      zoom += (mousePos.y - (float)posy) * .005f * zoomSpeed;
      camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)posy) * .005f * zoomSpeed));
      mousePos = glm::vec2((float)posx, (float)posy);
      viewUpdated = true;
    }
    if (mouseButtons.left) {
      rotation.x += (mousePos.y - (float)posy) * 1.25f * rotationSpeed;
      rotation.y -= (mousePos.x - (float)posx) * 1.25f * rotationSpeed;
      camera.rotate(glm::vec3((mousePos.y - (float)posy) * camera.rotationSpeed, -(mousePos.x - (float)posx) * camera.rotationSpeed, 0.0f));
      mousePos = glm::vec2((float)posx, (float)posy);
      viewUpdated = true;
    }
    if (mouseButtons.middle) {
      cameraPos.x -= (mousePos.x - (float)posx) * 0.01f;
      cameraPos.y -= (mousePos.y - (float)posy) * 0.01f;
      camera.translate(glm::vec3(-(mousePos.x - (float)posx) * 0.01f, -(mousePos.y - (float)posy) * 0.01f, 0.0f));
      mousePos = glm::vec2((float)posx, (float)posy);
      viewUpdated = true;
    }
    break;
  }
  case WM_SIZE:
    if ((prepared) && (wParam != SIZE_MINIMIZED))
    {
      if ((resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
      {
        destWidth = LOWORD(lParam);
        destHeight = HIWORD(lParam);
        windowResize();
      }
    }
    break;
  case WM_ENTERSIZEMOVE:
    resizing = true;
    break;
  case WM_EXITSIZEMOVE:
    resizing = false;
    break;
  }
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
int32_t VulkanBase::handleAppInput(struct android_app* app, AInputEvent* event)
{
  VulkanBase* vulkanApp = reinterpret_cast<VulkanBase*>(app->userData);
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
  {
    int32_t eventSource = AInputEvent_getSource(event);
    switch (eventSource) {
      case AINPUT_SOURCE_JOYSTICK: {
        // Left thumbstick
        vulkanApp->gamePadState.axisLeft.x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);
        vulkanApp->gamePadState.axisLeft.y = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);
        // Right thumbstick
        vulkanApp->gamePadState.axisRight.x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0);
        vulkanApp->gamePadState.axisRight.y = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0);
        break;
      }

      case AINPUT_SOURCE_TOUCHSCREEN: {
        int32_t action = AMotionEvent_getAction(event);

        switch (action) {
          case AMOTION_EVENT_ACTION_UP: {
            vulkanApp->lastTapTime = AMotionEvent_getEventTime(event);
            vulkanApp->touchPos.x = AMotionEvent_getX(event, 0);
            vulkanApp->touchPos.y = AMotionEvent_getY(event, 0);
            vulkanApp->touchTimer = 0.0;
            vulkanApp->touchDown = false;
            vulkanApp->camera.keys.up = false;
            return 1;
            break;
          }
          case AMOTION_EVENT_ACTION_DOWN: {
            vulkanApp->touchDown = true;
            vulkanApp->touchPos.x = AMotionEvent_getX(event, 0);
            vulkanApp->touchPos.y = AMotionEvent_getY(event, 0);
            break;
          }
          case AMOTION_EVENT_ACTION_MOVE: {
            int32_t eventX = AMotionEvent_getX(event, 0);
            int32_t eventY = AMotionEvent_getY(event, 0);

            float deltaX = (float)(vulkanApp->touchPos.y - eventY) * vulkanApp->rotationSpeed * 0.5f;
            float deltaY = (float)(vulkanApp->touchPos.x - eventX) * vulkanApp->rotationSpeed * 0.5f;

            vulkanApp->camera.rotate(glm::vec3(deltaX, 0.0f, 0.0f));
            vulkanApp->camera.rotate(glm::vec3(0.0f, -deltaY, 0.0f));

            vulkanApp->rotation.x += deltaX;
            vulkanApp->rotation.y -= deltaY;

            vulkanApp->viewChanged();

            vulkanApp->touchPos.x = eventX;
            vulkanApp->touchPos.y = eventY;
            break;
          }
          default:
            return 1;
            break;
        }
      }

      return 1;
    }
  }

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
  {
    int32_t keyCode = AKeyEvent_getKeyCode((const AInputEvent*)event);
    int32_t action = AKeyEvent_getAction((const AInputEvent*)event);
    int32_t button = 0;

    if (action == AKEY_EVENT_ACTION_UP){
      if(keyCode == KEY_BACKSPACE){ // AKEYCODE_BACKSPACE:
        vulkanApp->keyReleased(KEY_BACKSPACE);
      }
      else
      if(keyCode == AKEYCODE_BACK){
        vulkanApp->keyReleased(BACK_BUTTON);
        return 1;
      }
      return 0;
    }

    switch (keyCode)
    {
    case KEY_BACKSPACE: // AKEYCODE_BACKSPACE:
      vulkanApp->keyPressed(KEY_BACKSPACE, 0);
      break;
    case AKEYCODE_BACK:
      vulkanApp->keyPressed(BACK_BUTTON, 0);
      break;
    case AKEYCODE_BUTTON_A:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_A, 0);
      break;
    case AKEYCODE_BUTTON_B:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_B, 0);
      break;
    case AKEYCODE_BUTTON_X:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_X, 0);
      break;
    case AKEYCODE_BUTTON_Y:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_Y, 0);
      break;
    case AKEYCODE_BUTTON_L1:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_L1, 0);
      break;
    case AKEYCODE_BUTTON_R1:
      vulkanApp->keyPressed(GAMEPAD_BUTTON_R1, 0);
      break;
    case AKEYCODE_BUTTON_START:
      vulkanApp->paused = !vulkanApp->paused;
      break;
    };
  }

  return 0;
}

void VulkanBase::handleAppCommand(android_app * app, int32_t cmd)
{
  assert(app->userData != NULL);
  VulkanBase* vulkanApp = reinterpret_cast<VulkanBase*>(app->userData);
  switch (cmd)
  {
  case APP_CMD_SAVE_STATE:
    LOGD("APP_CMD_SAVE_STATE");
    /*
    vulkanApp->app->savedState = malloc(sizeof(struct saved_state));
    *((struct saved_state*)vulkanApp->app->savedState) = vulkanApp->state;
    vulkanApp->app->savedStateSize = sizeof(struct saved_state);
    */
    break;
  case APP_CMD_INIT_WINDOW:
    LOGD("APP_CMD_INIT_WINDOW");
    if (androidApp->window != NULL) {
      vulkanApp->startup();
      vulkanApp->prepare();
      assert(vulkanApp->prepared);
    }
    else LOGE("No window assigned!");
    break;
  case APP_CMD_LOST_FOCUS:
    LOGD("APP_CMD_LOST_FOCUS");
    vulkanApp->focused = false;
    break;
  case APP_CMD_GAINED_FOCUS:
    LOGD("APP_CMD_GAINED_FOCUS");
    vulkanApp->focused = true;
    break;
  case APP_CMD_TERM_WINDOW:
    // Window is hidden or closed, clean up resources
    LOGD("APP_CMD_TERM_WINDOW");
    vulkanApp->cleanup();
    break;
  }
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
void* VulkanBase::setupWindow(void* view)
{
  this->view = view;
  return view;
}
#elif defined(_DIRECT2DISPLAY)
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
/*static*/void VulkanBase::registryGlobalCb(void *data,
    wl_registry *registry, uint32_t name, const char *interface,
    uint32_t version)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->registryGlobal(registry, name, interface, version);
}

/*static*/void VulkanBase::seatCapabilitiesCb(void *data, wl_seat *seat,
    uint32_t caps)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->seatCapabilities(seat, caps);
}

/*static*/void VulkanBase::pointerEnterCb(void *data,
    wl_pointer *pointer, uint32_t serial, wl_surface *surface,
    wl_fixed_t sx, wl_fixed_t sy)
{
}

/*static*/void VulkanBase::pointerLeaveCb(void *data,
    wl_pointer *pointer, uint32_t serial, wl_surface *surface)
{
}

/*static*/void VulkanBase::pointerMotionCb(void *data,
    wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->pointerMotion(pointer, time, sx, sy);
}
void VulkanBase::pointerMotion(wl_pointer *pointer, uint32_t time,
    wl_fixed_t sx, wl_fixed_t sy)
{
  double x = wl_fixed_to_double(sx);
  double y = wl_fixed_to_double(sy);

  double dx = mousePos.x - x;
  double dy = mousePos.y - y;

  if (mouseButtons.left)
  {
    rotation.x += dy * 1.25f * rotationSpeed;
    rotation.y -= dx * 1.25f * rotationSpeed;
    camera.rotate(glm::vec3(
        dy * camera.rotationSpeed,
        -dx * camera.rotationSpeed,
        0.0f));
    viewUpdated = true;
  }
  if (mouseButtons.right)
  {
    zoom += dy * .005f * zoomSpeed;
    camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f * zoomSpeed));
    viewUpdated = true;
  }
  if (mouseButtons.middle)
  {
    cameraPos.x -= dx * 0.01f;
    cameraPos.y -= dy * 0.01f;
    camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
    viewUpdated = true;
  }
  mousePos = glm::vec2(x, y);
}

/*static*/void VulkanBase::pointerButtonCb(void *data,
    wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
    uint32_t state)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->pointerButton(pointer, serial, time, button, state);
}

void VulkanBase::pointerButton(struct wl_pointer *pointer,
    uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
  switch (button)
  {
  case BTN_LEFT:
    mouseButtons.left = !!state;
    break;
  case BTN_MIDDLE:
    mouseButtons.middle = !!state;
    break;
  case BTN_RIGHT:
    mouseButtons.right = !!state;
    break;
  default:
    break;
  }
}

/*static*/void VulkanBase::pointerAxisCb(void *data,
    wl_pointer *pointer, uint32_t time, uint32_t axis,
    wl_fixed_t value)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->pointerAxis(pointer, time, axis, value);
}

void VulkanBase::pointerAxis(wl_pointer *pointer, uint32_t time,
    uint32_t axis, wl_fixed_t value)
{
  double d = wl_fixed_to_double(value);
  switch (axis)
  {
  case REL_X:
    zoom += d * 0.005f * zoomSpeed;
    camera.translate(glm::vec3(0.0f, 0.0f, d * 0.005f * zoomSpeed));
    viewUpdated = true;
    break;
  default:
    break;
  }
}

/*static*/void VulkanBase::keyboardKeymapCb(void *data,
    struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
}

/*static*/void VulkanBase::keyboardEnterCb(void *data,
    struct wl_keyboard *keyboard, uint32_t serial,
    struct wl_surface *surface, struct wl_array *keys)
{
}

/*static*/void VulkanBase::keyboardLeaveCb(void *data,
    struct wl_keyboard *keyboard, uint32_t serial,
    struct wl_surface *surface)
{
}

/*static*/void VulkanBase::keyboardKeyCb(void *data,
    struct wl_keyboard *keyboard, uint32_t serial, uint32_t time,
    uint32_t key, uint32_t state)
{
  VulkanBase *self = reinterpret_cast<VulkanBase *>(data);
  self->keyboardKey(keyboard, serial, time, key, state);
}

void VulkanBase::keyboardKey(struct wl_keyboard *keyboard,
    uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
  switch (key)
  {
  case KEY_W:
    camera.keys.up = !!state;
    break;
  case KEY_S:
    camera.keys.down = !!state;
    break;
  case KEY_A:
    camera.keys.left = !!state;
    break;
  case KEY_D:
    camera.keys.right = !!state;
    break;
  case KEY_P:
    if (state)
      paused = !paused;
    break;
  case KEY_ESC:
    quit = true;
    break;
  }

  if (state)
    keyPressed(key, 0);
}

/*static*/void VulkanBase::keyboardModifiersCb(void *data,
    struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
    uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
}

void VulkanBase::seatCapabilities(wl_seat *seat, uint32_t caps)
{
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer)
  {
    pointer = wl_seat_get_pointer(seat);
    static const struct wl_pointer_listener pointer_listener =
    { pointerEnterCb, pointerLeaveCb, pointerMotionCb, pointerButtonCb,
        pointerAxisCb, };
    wl_pointer_add_listener(pointer, &pointer_listener, this);
  }
  else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && pointer)
  {
    wl_pointer_destroy(pointer);
    pointer = nullptr;
  }

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard)
  {
    keyboard = wl_seat_get_keyboard(seat);
    static const struct wl_keyboard_listener keyboard_listener =
    { keyboardKeymapCb, keyboardEnterCb, keyboardLeaveCb, keyboardKeyCb,
        keyboardModifiersCb, };
    wl_keyboard_add_listener(keyboard, &keyboard_listener, this);
  }
  else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && keyboard)
  {
    wl_keyboard_destroy(keyboard);
    keyboard = nullptr;
  }
}

void VulkanBase::registryGlobal(wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version)
{
  if (strcmp(interface, "wl_compositor") == 0)
  {
    compositor = (wl_compositor *) wl_registry_bind(registry, name,
        &wl_compositor_interface, 3);
  }
  else if (strcmp(interface, "wl_shell") == 0)
  {
    shell = (wl_shell *) wl_registry_bind(registry, name,
        &wl_shell_interface, 1);
  }
  else if (strcmp(interface, "wl_seat") == 0)
  {
    seat = (wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface,
        1);

    static const struct wl_seat_listener seat_listener =
    { seatCapabilitiesCb, };
    wl_seat_add_listener(seat, &seat_listener, this);
  }
}

/*static*/void VulkanBase::registryGlobalRemoveCb(void *data,
    struct wl_registry *registry, uint32_t name)
{
}

void VulkanBase::initWaylandConnection()
{
  display = wl_display_connect(NULL);
  if (!display)
  {
    std::cout << "Could not connect to Wayland display!\n";
    fflush(stdout);
    exit(1);
  }

  registry = wl_display_get_registry(display);
  if (!registry)
  {
    std::cout << "Could not get Wayland registry!\n";
    fflush(stdout);
    exit(1);
  }

  static const struct wl_registry_listener registry_listener =
  { registryGlobalCb, registryGlobalRemoveCb };
  wl_registry_add_listener(registry, &registry_listener, this);
  wl_display_dispatch(display);
  wl_display_roundtrip(display);
  if (!compositor || !shell || !seat)
  {
    std::cout << "Could not bind Wayland protocols!\n";
    fflush(stdout);
    exit(1);
  }
}

static void PingCb(void *data, struct wl_shell_surface *shell_surface,
    uint32_t serial)
{
  wl_shell_surface_pong(shell_surface, serial);
}

static void ConfigureCb(void *data, struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height)
{
}

static void PopupDoneCb(void *data, struct wl_shell_surface *shell_surface)
{
}

wl_shell_surface *VulkanBase::setupWindow()
{
  surface = wl_compositor_create_surface(compositor);
  shell_surface = wl_shell_get_shell_surface(shell, surface);

  static const struct wl_shell_surface_listener shell_surface_listener =
  { PingCb, ConfigureCb, PopupDoneCb };

  wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, this);
  wl_shell_surface_set_toplevel(shell_surface);
  std::string windowTitle = getWindowTitle();
  wl_shell_surface_set_title(shell_surface, windowTitle.c_str());
  return shell_surface;
}

#elif defined(VK_USE_PLATFORM_XCB_KHR)

static inline xcb_intern_atom_reply_t* intern_atom_helper(xcb_connection_t *conn, bool only_if_exists, const char *str)
{
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
  return xcb_intern_atom_reply(conn, cookie, NULL);
}

// Set up a window using XCB and request event types
xcb_window_t VulkanBase::setupWindow()
{
  uint32_t value_mask, value_list[32];

  window = xcb_generate_id(connection);

  value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  value_list[0] = screen->black_pixel;
  value_list[1] =
    XCB_EVENT_MASK_KEY_RELEASE |
    XCB_EVENT_MASK_KEY_PRESS |
    XCB_EVENT_MASK_EXPOSURE |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
    XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_BUTTON_PRESS |
    XCB_EVENT_MASK_BUTTON_RELEASE;

  if (settings.fullscreen)
  {
    width = destWidth = screen->width_in_pixels;
    height = destHeight = screen->height_in_pixels;
  }

  xcb_create_window(connection,
    XCB_COPY_FROM_PARENT,
    window, screen->root,
    0, 0, width, height, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    value_mask, value_list);

  /* Magic code that will send notification when window is destroyed */
  xcb_intern_atom_reply_t* reply = intern_atom_helper(connection, true, "WM_PROTOCOLS");
  atom_wm_delete_window = intern_atom_helper(connection, false, "WM_DELETE_WINDOW");

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
    window, (*reply).atom, 4, 32, 1,
    &(*atom_wm_delete_window).atom);

  std::string windowTitle = getWindowTitle();
  xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
    window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
    title.size(), windowTitle.c_str());

  free(reply);

  if (settings.fullscreen)
  {
    xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper(connection, false, "_NET_WM_STATE");
    xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper(connection, false, "_NET_WM_STATE_FULLSCREEN");
    xcb_change_property(connection,
        XCB_PROP_MODE_REPLACE,
        window, atom_wm_state->atom,
        XCB_ATOM_ATOM, 32, 1,
        &(atom_wm_fullscreen->atom));
    free(atom_wm_fullscreen);
    free(atom_wm_state);
  }

  xcb_map_window(connection, window);

  return(window);
}

struct xkb_rule_names rns = {
  rules: "evdev",
  model: "macbook78",
  layout: "gb",
  variant: NULL,
  options: NULL  // "ctrl:swapcaps"; "ctrl:nocaps"
};
struct xkb_context* ctx;
struct xkb_keymap* keymap;
struct xkb_state* state;

// Initialize XCB connection
void VulkanBase::initxcbConnection()
{
  const xcb_setup_t *setup;
  xcb_screen_iterator_t iter;
  int scr;

  connection = xcb_connect(NULL, &scr);
  if (connection == NULL) {
    printf("Could not find a compatible Vulkan ICD!\n");
    fflush(stdout);
    exit(1);
  }

  setup = xcb_get_setup(connection);
  iter = xcb_setup_roots_iterator(setup);
  while (scr-- > 0)
    xcb_screen_next(&iter);
  screen = iter.data;
  ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);                                if (!ctx)    LOG("oops, dodgy context\n");
  keymap = xkb_keymap_new_from_names(ctx, &rns, XKB_KEYMAP_COMPILE_NO_FLAGS); if (!keymap) LOG("oops, dodgy keymap\n");
  state = xkb_state_new(keymap);                                              if (!state)  LOG("oops, dodgy state\n");
}

void VulkanBase::handleEvent(const xcb_generic_event_t *event)
{
  switch (event->response_type & 0x7f)
  {
  case XCB_CLIENT_MESSAGE:
    if ((*(xcb_client_message_event_t*)event).data.data32[0] ==
      (*atom_wm_delete_window).atom) {
      quit = true;
    }
    break;
  case XCB_MOTION_NOTIFY:
  {
    xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;
    if (mouseButtons.left)
    {
      rotation.x += (mousePos.y - (float)motion->event_y) * 1.25f;
      rotation.y -= (mousePos.x - (float)motion->event_x) * 1.25f;
      camera.rotate(glm::vec3((mousePos.y - (float)motion->event_y) * camera.rotationSpeed, -(mousePos.x - (float)motion->event_x) * camera.rotationSpeed, 0.0f));
      viewUpdated = true;
    }
    if (mouseButtons.right)
    {
      zoom += (mousePos.y - (float)motion->event_y) * .005f;
      camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)motion->event_y) * .005f * zoomSpeed));
      viewUpdated = true;
    }
    if (mouseButtons.middle)
    {
      cameraPos.x -= (mousePos.x - (float)motion->event_x) * 0.01f;
      cameraPos.y -= (mousePos.y - (float)motion->event_y) * 0.01f;
      camera.translate(glm::vec3(-(mousePos.x - (float)(float)motion->event_x) * 0.01f, -(mousePos.y - (float)motion->event_y) * 0.01f, 0.0f));
      viewUpdated = true;
      mousePos.x = (float)motion->event_x;
      mousePos.y = (float)motion->event_y;
    }
    mousePos = glm::vec2((float)motion->event_x, (float)motion->event_y);
  }
  break;
  case XCB_BUTTON_PRESS:
  {
    xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
    if (press->detail == XCB_BUTTON_INDEX_1)
      mouseButtons.left = true;
    if (press->detail == XCB_BUTTON_INDEX_2)
      mouseButtons.middle = true;
    if (press->detail == XCB_BUTTON_INDEX_3)
      mouseButtons.right = true;
    if (press->detail == XCB_BUTTON_INDEX_4)
      mouseButtons.wheelDown = true;
    if (press->detail == XCB_BUTTON_INDEX_5)
      mouseButtons.wheelUp = true;
  }
  break;
  case XCB_BUTTON_RELEASE:
  {
    xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
    if (press->detail == XCB_BUTTON_INDEX_1)
      mouseButtons.left = false;
    if (press->detail == XCB_BUTTON_INDEX_2)
      mouseButtons.middle = false;
    if (press->detail == XCB_BUTTON_INDEX_3)
      mouseButtons.right = false;
  }
  break;
  case XCB_KEY_PRESS:
  {
    const xcb_key_press_event_t* keyEvent = (const xcb_key_press_event_t*)event;
    xcb_keycode_t keyCode = keyEvent->detail;
    if(keyCode==0x42) keyCode=0x25;
    else
    if(keyCode==0x25) keyCode=0x42;
    switch (keyCode)
    {
      case KEY_W:
        camera.keys.up = true;
        break;
      case KEY_S:
        camera.keys.down = true;
        break;
      case KEY_A:
        camera.keys.left = true;
        break;
      case KEY_D:
        camera.keys.right = true;
        break;
      case KEY_P:
        paused = !paused;
        break;
    }
    xkb_state_update_key(state, keyCode, XKB_KEY_DOWN);
    char32_t u32key = xkb_state_key_get_utf32(state, keyCode);
    keyPressed(keyCode, u32key);
  }
  break;
  case XCB_KEY_RELEASE:
  {
    const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*)event;
    xcb_keycode_t keyCode = keyEvent->detail;
    if(keyCode==0x42)keyCode=0x25;
    else
    if(keyCode==0x25)keyCode=0x42;
    switch (keyCode)
    {
      case KEY_W:
        camera.keys.up = false;
        break;
      case KEY_S:
        camera.keys.down = false;
        break;
      case KEY_A:
        camera.keys.left = false;
        break;
      case KEY_D:
        camera.keys.right = false;
        break;
      case KEY_ESCAPE:
        quit = true;
        break;
    }
    xkb_state_update_key(state, keyCode, XKB_KEY_UP);
    keyReleased(keyCode);
  }
  break;
  case XCB_DESTROY_NOTIFY:
    quit = true;
    break;
  case XCB_CONFIGURE_NOTIFY:
  {
    const xcb_configure_notify_event_t *cfgEvent = (const xcb_configure_notify_event_t *)event;
    if ((prepared) && ((cfgEvent->width != width) || (cfgEvent->height != height)))
    {
        destWidth = cfgEvent->width;
        destHeight = cfgEvent->height;
        if ((destWidth > 0) && (destHeight > 0))
        {
          windowResize();
        }
    }
  }
  break;
  default:
    break;
  }
}
#endif

void VulkanBase::viewChanged() {}

void VulkanBase::keyPressed(uint32_t, char32_t) {}

void VulkanBase::keyReleased(uint32_t) {}

void VulkanBase::mouseMoved(double x, double y, bool & handled) {}

void VulkanBase::buildCommandBuffers() {}

void VulkanBase::createCommandPool()
{
  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
  cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}

void VulkanBase::setupDepthStencil()
{
  VkImageCreateInfo image = {};
  image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image.pNext = NULL;
  image.imageType = VK_IMAGE_TYPE_2D;
  image.format = depthFormat;
  image.extent = { width, height, 1 };
  image.mipLevels = 1;
  image.arrayLayers = 1;
  image.samples = VK_SAMPLE_COUNT_1_BIT;
  image.tiling = VK_IMAGE_TILING_OPTIMAL;
  image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  image.flags = 0;

  VkMemoryAllocateInfo mem_alloc = {};
  mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_alloc.pNext = NULL;
  mem_alloc.allocationSize = 0;
  mem_alloc.memoryTypeIndex = 0;

  VkImageViewCreateInfo depthStencilView = {};
  depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depthStencilView.pNext = NULL;
  depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depthStencilView.format = depthFormat;
  depthStencilView.flags = 0;
  depthStencilView.subresourceRange = {};
  depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  depthStencilView.subresourceRange.baseMipLevel = 0;
  depthStencilView.subresourceRange.levelCount = 1;
  depthStencilView.subresourceRange.baseArrayLayer = 0;
  depthStencilView.subresourceRange.layerCount = 1;

  VkMemoryRequirements memReqs;

  VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthStencil.image));
  vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);
  mem_alloc.allocationSize = memReqs.size;
  mem_alloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK_RESULT(vkAllocateMemory(device, &mem_alloc, nullptr, &depthStencil.mem));
  VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));

  depthStencilView.image = depthStencil.image;
  VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthStencil.view));
}

void VulkanBase::setupFrameBuffer()
{
  VkImageView attachments[2];

  // Depth/Stencil attachment is the same for all frame buffers
  attachments[1] = depthStencil.view;

  VkFramebufferCreateInfo frameBufferCreateInfo = {};
  frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  frameBufferCreateInfo.pNext = NULL;
  frameBufferCreateInfo.renderPass = renderPass;
  frameBufferCreateInfo.attachmentCount = 2;
  frameBufferCreateInfo.pAttachments = attachments;
  frameBufferCreateInfo.width = width;
  frameBufferCreateInfo.height = height;
  frameBufferCreateInfo.layers = 1;

  // Create frame buffers for every swap chain image
  frameBuffers.resize(swapChain.imageCount);
  for (uint32_t i = 0; i < frameBuffers.size(); i++)
  {
    attachments[0] = swapChain.buffers[i].view;
    VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
  }
}

void VulkanBase::setupRenderPass()
{
  std::array<VkAttachmentDescription, 2> attachments = {};
  // Color attachment
  attachments[0].format = swapChain.colorFormat;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  // Depth attachment
  attachments[1].format = depthFormat;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthReference = {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;
  subpassDescription.pDepthStencilAttachment = &depthReference;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  subpassDescription.pResolveAttachments = nullptr;

  // Subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies;

  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpassDescription;
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();

  VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void VulkanBase::getEnabledFeatures()
{
  // Can be overriden in derived class
}

void VulkanBase::windowResize()
{
  if (!prepared)
  {
    return;
  }
  prepared = false;

  // Ensure all operations on the device have been finished before destroying resources
  vkDeviceWaitIdle(device);

  // Recreate swap chain
  width = destWidth;
  height = destHeight;
  setupSwapChain();

  // Recreate the frame buffers

  vkDestroyImageView(device, depthStencil.view, nullptr);
  vkDestroyImage(device, depthStencil.image, nullptr);
  vkFreeMemory(device, depthStencil.mem, nullptr);
  setupDepthStencil();

  for (uint32_t i = 0; i < frameBuffers.size(); i++)
  {
    vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
  }
  setupFrameBuffer();

  // Command buffers need to be recreated as they may store
  // references to the recreated frame buffer
  destroyCommandBuffers();
  createCommandBuffers();
  buildCommandBuffers();

  vkDeviceWaitIdle(device);

  camera.updateAspectRatio((float)width / (float)height);

  // Notify derived class
  windowResized();
  viewChanged();

  prepared = true;
}

void VulkanBase::windowResized()
{
  // Can be overriden in derived class
}

void VulkanBase::initSwapchain()
{
#if defined(_WIN32)
  swapChain.initSurface(windowInstance, window);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  swapChain.initSurface(androidApp->window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
  swapChain.initSurface(view);
#elif defined(_DIRECT2DISPLAY)
  swapChain.initSurface(width, height);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  swapChain.initSurface(display, surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  swapChain.initSurface(connection, window);
#endif
}

void VulkanBase::setupSwapChain()
{
  swapChain.create(&width, &height, settings.vsync);
}
