
#include "gui.h"
extern "C" {
#include <onex-kernel/log.h>
#include <onf.h>
}

#define ENABLE_VALIDATION false

static GUI* static_gui;

object* user;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
extern "C"
{
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jobject thiz, jint keyCode, jstring key);
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyRelease(JNIEnv* env, jobject thiz, jint keyCode);
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onSerialRecv(JNIEnv* env, jobject thiz, jstring b);
};

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jobject thiz, jint keyCode, jstring key)
{
  jchar keychars[1]; env->GetStringRegion(key, 0, 1, keychars);
  char32_t u32key = keychars[0];
  if(u32key!=' ' || keyCode==KEY_SPACE) static_gui->keyPressed(keyCode, u32key);
  else                                  static_gui->keyPressed(keyCode, 0);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyRelease(JNIEnv* env, jobject thiz, jint keyCode)
{
  static_gui->keyReleased(keyCode);
}

extern "C" {
void on_serial_recv(char* b);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onSerialRecv(JNIEnv* env, jobject thiz, jstring b)
{
  const char* chars = env->GetStringUTFChars(b, 0);
  on_serial_recv((char*)chars);
  env->ReleaseStringUTFChars(b, chars);
}
#endif

bool keyboardUp = false;

void showOrHideSoftKeyboard(bool show)
{
  onex_run_evaluators(user);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  if(keyboardUp == show) return;
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, show? "showKeyboard": "hideKeyboard", "()V");
  env->CallVoidMethod(nativeActivity, method);
  androidApp->activity->vm->DetachCurrentThread();
  keyboardUp = show;
#endif
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
extern "C" {

void serial_send(char* b)
{
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "serialSend", "(Ljava/lang/String;)V");
  jstring buff = env->NewStringUTF(b);
  env->CallVoidMethod(nativeActivity, method, buff);
  env->DeleteLocalRef(buff);
  androidApp->activity->vm->DetachCurrentThread();
}

}
#endif

class OnexApp : public VulkanBase
{
  GUI* gui;

public:

  static bool evaluate_user(object* user)
  {
    return true;
  }

  static bool evaluate_list(object* user)
  {
    return true;
  }

  OnexApp() : VulkanBase(ENABLE_VALIDATION)
  {
    title = "Vulkan App";

    timerSpeed *= 8.0f;
    srand(time(NULL));

    camera.position = { 10.0f, -13.5f, 0.0f };
    camera.setRotation(glm::vec3(5.0f, 90.0f, 0.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);

    onex_init();

    object* art1p1=object_new_from(strdup("is: word list  list: Welcome to Onex! First thing to do is to go around trying all the buttons .."), 0, 2);
    object* art1p2=object_new_from(strdup("is: word list  list: Then try making yourself a todo list.."), 0, 2);

    object* art1=object_new_from(strdup("is: article title: First steps.. "), 0, 4);
    object_property_set(art1, (char*)"text", object_property(art1p1, (char*)"UID"));
    object_property_add(art1, (char*)"text", object_property(art1p2, (char*)"UID"));

    object* art2p1=object_new_from(strdup("is: word list  list: Now that you've had a chance to try things out.."), 0, 2);
    object* art2p2=object_new_from(strdup("is: word list  list: You could connect your Micro:Bit.."), 0, 2);

    object* art2=object_new_from(strdup("is: article title: Going deeper.. "), 0, 4);
    object_property_set(art2, (char*)"text", object_property(art2p1, (char*)"UID"));
    object_property_add(art2, (char*)"text", object_property(art2p2, (char*)"UID"));

    object* intro=object_new(0, (char*)"article list", evaluate_list, 4);
    object_property_set(intro, (char*)"title", (char*)"Introduction to Onex, the live personal database");
    object_property_set(intro, (char*)"list", object_property(art1, (char*)"UID"));
    object_property_add(intro, (char*)"list", object_property(art2, (char*)"UID"));

    object* famp1=object_new_from(strdup("is: person  name: Joe Banana"), evaluate_list, 20);
    object* famp2=object_new_from(strdup("is: person  name: Ann Mango"), evaluate_list, 20);

    object_property_set(famp1, (char*)"partner", object_property(famp2, (char*)"UID"));
    object_property_set(famp2, (char*)"partner", object_property(famp1, (char*)"UID"));

    object* famtree=object_new_from(strdup("is: family tree  title: My family  "), evaluate_list, 20);
    object_property_set(famtree, (char*)"people", object_property(famp1, (char*)"UID"));
    object_property_add(famtree, (char*)"people", object_property(famp2, (char*)"UID"));

    object* links=object_new((char*)"uid-2", (char*)"links list", evaluate_list, 4);
    object_property_set(links, (char*)"list", object_property(intro, (char*)"UID"));
    object_property_add(links, (char*)"list", object_property(famtree, (char*)"UID"));
//  object_property_add(links, (char*)"list", (char*)"uid-1-2-3");

    user=object_new((char*)"uid-1", (char*)"user", evaluate_user, 4);
    object_property_set(user, (char*)"viewing-l", (char*)"uid-2");
    object_property_set(user, (char*)"viewing-r", (char*)"uid-2");

    gui = new GUI(this, user);
    static_gui = gui;
  }

  virtual void startup()
  {
    VulkanBase::startup();
    if(!gui) gui = new GUI(this, user);
    static_gui = gui;
  }

  virtual void cleanup()
  {
    if(keyboardUp){ showOrHideSoftKeyboard(false); keyboardUp = true; }
    delete gui; gui=0;
    VulkanBase::cleanup();
  }

  ~OnexApp()
  {
    delete gui;
  }

  void prepare()
  {
    log_write("OnexApp----------------------\n");
    VulkanBase::prepare();
    gui->prepare();
    buildCommandBuffers();
    prepared = true;
    if(keyboardUp){ keyboardUp = false; showOrHideSoftKeyboard(true); }
  }

  void buildCommandBuffers()
  {
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[0].color = { { 0.25f, 0.25f, 0.25f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
      renderPassBeginInfo.framebuffer = frameBuffers[i];

      VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));
      {
        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
          VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
          vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

          VkRect2D scissor = vks::initializers::rect2D(width, height, 0,0);
          vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

          gui->buildCommandBuffers(i);
        }
        vkCmdEndRenderPass(drawCmdBuffers[i]);
      }
      VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
  }

  virtual void render()
  {
    if (!prepared) return;

    onex_loop();

    gui->render();

    reBuildCommandBuffers(); // rebuild all cmdbufs every frame!!

    VulkanBase::prepareFrame();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VulkanBase::submitFrame();
  }

  virtual void viewChanged()
  {
    gui->updateUniformBuffers();
  }

  virtual void getEnabledFeatures()
  {
    if (deviceFeatures.fillModeNonSolid) {
      enabledFeatures.fillModeNonSolid = VK_TRUE;
    };
    if (deviceFeatures.samplerAnisotropy) {
      enabledFeatures.samplerAnisotropy = VK_TRUE;
    }
    if (deviceFeatures.textureCompressionBC) {
      enabledFeatures.textureCompressionBC = VK_TRUE;
    }
    else if (deviceFeatures.textureCompressionASTC_LDR) {
      enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
    }
    else if (deviceFeatures.textureCompressionETC2) {
      enabledFeatures.textureCompressionETC2 = VK_TRUE;
    }
  }

  virtual void keyPressed(uint32_t keyCode, char32_t u32key)
  {
    gui->keyPressed(keyCode, u32key);
  }

  virtual void keyReleased(uint32_t keyCode)
  {
    gui->keyReleased(keyCode);
  }
};


// OS specific macros for the example main entry points
#if defined(_WIN32)
// Windows entry point
#define VULKAN_EXAMPLE_MAIN()                                    \
OnexApp *vulkanApp;                                    \
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)            \
{                                                  \
  if (vulkanApp != NULL)                                    \
  {                                                \
    vulkanApp->handleMessages(hWnd, uMsg, wParam, lParam);                  \
  }                                                \
  return (DefWindowProc(hWnd, uMsg, wParam, lParam));                        \
}                                                  \
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)                  \
{                                                  \
  for (int32_t i = 0; i < __argc; i++) { OnexApp::args.push_back(__argv[i]); };        \
  vulkanApp = new OnexApp();                              \
  vulkanApp->initVulkan();                                  \
  vulkanApp->setupWindow(hInstance, WndProc);                          \
  vulkanApp->initSwapchain();                                  \
  vulkanApp->prepare();                                    \
  vulkanApp->renderLoop();                                  \
  delete(vulkanApp);                                      \
  return 0;                                            \
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android entry point
#define VULKAN_EXAMPLE_MAIN()                                    \
OnexApp *vulkanApp;                                    \
void android_main(android_app* state)                                \
{                                                  \
  vulkanApp = new OnexApp();                              \
  state->userData = vulkanApp;                                \
  state->onAppCmd = OnexApp::handleAppCommand;                        \
  state->onInputEvent = OnexApp::handleAppInput;                      \
  androidApp = state;                                        \
  vks::android::getDeviceConfig();                                \
  vulkanApp->renderLoop();                                  \
  delete(vulkanApp);                                      \
}
#elif defined(_DIRECT2DISPLAY)
// Linux entry point with direct to display wsi
#define VULKAN_EXAMPLE_MAIN()                                    \
OnexApp *vulkanApp;                                    \
static void handleEvent()                                                      \
{                                                  \
}                                                  \
int main(const int argc, const char *argv[])                              \
{                                                  \
  for (size_t i = 0; i < argc; i++) { OnexApp::args.push_back(argv[i]); };          \
  vulkanApp = new OnexApp();                              \
  vulkanApp->initVulkan();                                  \
  vulkanApp->initSwapchain();                                  \
  vulkanApp->prepare();                                    \
  vulkanApp->renderLoop();                                  \
  delete(vulkanApp);                                      \
  return 0;                                            \
}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#define VULKAN_EXAMPLE_MAIN()                                    \
OnexApp *vulkanApp;                                    \
int main(const int argc, const char *argv[])                              \
{                                                  \
  for (size_t i = 0; i < argc; i++) { OnexApp::args.push_back(argv[i]); };          \
  vulkanApp = new OnexApp();                              \
  vulkanApp->initVulkan();                                  \
  vulkanApp->setupWindow();                                   \
  vulkanApp->initSwapchain();                                  \
  vulkanApp->prepare();                                    \
  vulkanApp->renderLoop();                                  \
  delete(vulkanApp);                                      \
  return 0;                                            \
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define VULKAN_EXAMPLE_MAIN()                                    \
OnexApp *vulkanApp;                                    \
static void handleEvent(const xcb_generic_event_t *event)                      \
{                                                  \
  if (vulkanApp != NULL)                                    \
  {                                                \
    vulkanApp->handleEvent(event);                              \
  }                                                \
}                                                  \
int main(const int argc, const char *argv[])                              \
{                                                  \
  for (size_t i = 0; i < argc; i++) { OnexApp::args.push_back(argv[i]); };          \
  vulkanApp = new OnexApp();                              \
  vulkanApp->initVulkan();                                  \
  vulkanApp->setupWindow();                                   \
  vulkanApp->initSwapchain();                                  \
  vulkanApp->prepare();                                    \
  vulkanApp->renderLoop();                                  \
  delete(vulkanApp);                                      \
  return 0;                                            \
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#define VULKAN_EXAMPLE_MAIN()
#endif

VULKAN_EXAMPLE_MAIN()
