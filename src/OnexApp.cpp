
#include "gui.h"
extern "C" {
#include <onex-kernel/log.h>
#include <onf.h>
}

#define ENABLE_VALIDATION false

static GUI* static_gui;

object* config;
object* user;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
extern "C"
{
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jobject thiz, jint keyCode, jint key);
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyRelease(JNIEnv* env, jobject thiz, jint keyCode);
  JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onSerialRecv(JNIEnv* env, jobject thiz, jstring b);
};

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jobject thiz, jint keyCode, jint u32key)
{
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

void sprintExternalStorageDirectory(char* buf, int buflen, const char* format)
{
  JNIEnv* env; androidApp->activity->vm->AttachCurrentThread(&env, 0);

  jclass osEnvClass = env->FindClass("android/os/Environment");
  jmethodID getExternalStorageDirectoryMethod = env->GetStaticMethodID(osEnvClass, "getExternalStorageDirectory", "()Ljava/io/File;");
  jobject extStorage = env->CallStaticObjectMethod(osEnvClass, getExternalStorageDirectoryMethod);

  jclass extStorageClass = env->GetObjectClass(extStorage);
  jmethodID getAbsolutePathMethod = env->GetMethodID(extStorageClass, "getAbsolutePath", "()Ljava/lang/String;");
  jstring extStoragePath = (jstring)env->CallObjectMethod(extStorage, getAbsolutePathMethod);

  const char* extStoragePathString=env->GetStringUTFChars(extStoragePath, 0);
  snprintf(buf, buflen, format, extStoragePathString);
  env->ReleaseStringUTFChars(extStoragePath, extStoragePathString);

  androidApp->activity->vm->DetachCurrentThread();
}
#endif

bool keyboardUp = false;

#define TEXTTYPE 1

void showOrHideSoftKeyboard(bool show)
{
  onex_run_evaluators(user);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  if(keyboardUp == show) return;
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  if(show){
    jmethodID method = env->GetMethodID(nativeActivityClass, "showKeyboard", "(I)V");
    env->CallVoidMethod(nativeActivity, method, TEXTTYPE);
  }else{
    jmethodID method = env->GetMethodID(nativeActivityClass, "hideKeyboard", "()V");
    env->CallVoidMethod(nativeActivity, method);
  }
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

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    char dbpath[128];
    sprintExternalStorageDirectory(dbpath, 128, "%s/Onex/onex.ondb");
    onex_init(dbpath);
#else
    onex_init((char*)"Onex/onex.ondb");
#endif

    onex_set_default_evaluator(evaluate_list);

    config=onex_get_from_cache((char*)"uid-0");

    if(!config){
      // UTF-8 hex:  "\xF0\x9F\x98\x83  \xF0\x9F\x93\xA6"

      object* tagbirth=object_new_from((char*)"is: tag  title: birthday  icon: 📦  colour: red", 0,5);
      object* tagparty=object_new_from((char*)"is: tag  title: party     icon: 🎉  colour: yellow", 0,5);
      object* tagtrain=object_new_from((char*)"is: tag  title: train     icon: 🚆  colour: blue", 0,5);
      object* tagceleb=object_new_from((char*)"is: tag  title: celebrate icon: 🎉  colour: red", 0,5);
      object* taglove =object_new_from((char*)"is: tag  title: love      icon: 😍  colour: red", 0,5);

      object* taglookup=object_new_from((char*)"is: tag lookup", 0, 100);
      object_property_set(taglookup, (char*)"birthday",  object_property(tagbirth, (char*)"UID"));
      object_property_set(taglookup, (char*)"party",     object_property(tagparty, (char*)"UID"));
      object_property_set(taglookup, (char*)"train",     object_property(tagtrain, (char*)"UID"));
      object_property_set(taglookup, (char*)"celebrate", object_property(tagceleb, (char*)"UID"));
      object_property_set(taglookup, (char*)"love",      object_property(taglove,  (char*)"UID"));

      object* links=object_new(0, (char*)"links list", evaluate_list, 4);
      object_property_set(links, (char*)"list", object_property(taglookup, (char*)"UID"));

      user=object_new(0, (char*)"user", evaluate_user, 8);
      object_property_set(user, (char*)"viewing-l", object_property(links, (char*)"UID"));

      config=object_new((char*)"uid-0", (char*)"config", 0, 10);
      object_property_set(config, (char*)"user",      object_property(user, (char*)"UID"));
      object_property_set(config, (char*)"taglookup", object_property(taglookup, (char*)"UID"));
    }
    else{
      char* userid=object_property(config, (char*)"user");
      user=onex_get_from_cache(userid);
    }
    gui = new GUI(this, user, config);
    static_gui = gui;
  }

  virtual void startup()
  {
    VulkanBase::startup();
    if(!gui) gui = new GUI(this, user, config);
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
  androidApp = state;                                        \
  vulkanApp = new OnexApp();                              \
  state->userData = vulkanApp;                                \
  state->onAppCmd = OnexApp::handleAppCommand;                        \
  state->onInputEvent = OnexApp::handleAppInput;                      \
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
