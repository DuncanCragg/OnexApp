
#include "gui.h"

#define ENABLE_VALIDATION false

static GUI* static_gui;

bool keyboardUp = false;

#define TEXTTYPE 1

extern "C" {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)

void onexInitialised(char* blemac)
{
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "onexInitialised", "(Ljava/lang/String;)V");
  jstring jblemac  = env->NewStringUTF(blemac);
  env->CallVoidMethod(nativeActivity, method, jblemac);
  env->DeleteLocalRef(jblemac);
  androidApp->activity->vm->DetachCurrentThread();
}

void showOrHideSoftKeyboard(bool show)
{
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
}

void serial_send(char* b)
{
  JavaVM* vm = androidApp->activity->vm;
  JNIEnv* env;
  jint res = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
  bool attached=false;
  if(res!=JNI_OK) {
     res=vm->AttachCurrentThread(&env, 0);
     if(res!=JNI_OK){
         log_write("Failed to AttachCurrentThread, ErrorCode: %d sending: %s", res, b);
         return;
     }
     attached=true;
  }
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "serialSend", "(Ljava/lang/String;)V");
  jstring buff = env->NewStringUTF(b);
  env->CallVoidMethod(nativeActivity, method, buff);
  env->DeleteLocalRef(buff);
  if(attached) vm->DetachCurrentThread();
}

void showNotification(char* title, char* text)
{
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "showNotification", "(Ljava/lang/String;Ljava/lang/String;)V");
  jstring jtitle = env->NewStringUTF(title);
  jstring jtext  = env->NewStringUTF(text);
  env->CallVoidMethod(nativeActivity, method, jtitle, jtext);
  env->DeleteLocalRef(jtitle); env->DeleteLocalRef(jtext);
  androidApp->activity->vm->DetachCurrentThread();
}

void setAlarm(time_t when, char* uid)
{
  log_write("setAlarm %ld %s\n", when, uid);
  JNIEnv* env;
  androidApp->activity->vm->AttachCurrentThread(&env, 0);
  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "setAlarm", "(JLjava/lang/String;)V");
  jlong jwhen = (jlong)when;
  jstring juid  = env->NewStringUTF(uid);
  env->CallVoidMethod(nativeActivity, method, jwhen, juid);
  env->DeleteLocalRef(juid);
  androidApp->activity->vm->DetachCurrentThread();
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

#else
void onexInitialised(char* blemac)
{
}

void showOrHideSoftKeyboard(bool show)
{
}

void showNotification(char* title, char* text)
{
  log_write("NOTIFICATION!!!! %s %s\n", title, text);
}

void setAlarm(time_t when, char* uid)
{
  log_write("setAlarm %ld %s\n", when, uid);
}
#endif
}

static char* pendingAlarmUID=0;

extern char* init_onex();

class OnexApp : public VulkanBase
{
  GUI* gui;
  bool onex_initialised=false;

public:

  OnexApp() : VulkanBase(ENABLE_VALIDATION)
  {
    title = "Vulkan App";

    timerSpeed *= 8.0f;
    srand(time(NULL));

    camera.position = { 10.0f, -13.5f, 0.0f };
    camera.setRotation(glm::vec3(5.0f, 90.0f, 0.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);

    gui = new GUI(this);
    static_gui = gui;
  }

  virtual void startup()
  {
    VulkanBase::startup();
    if(!gui) gui = new GUI(this);
    static_gui = gui;
  }

  virtual void cleanup()
  {
    if(keyboardUp){ showOrHideSoftKeyboard(false); keyboardUp = true; }
    delete gui; gui=0; static_gui=0;
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
    char* blemac=0;
    if(!onex_initialised){
      blemac=init_onex();
    }
    buildCommandBuffers();
    prepared = true;
    if(!onex_initialised){
      onexInitialised(blemac);
      onex_initialised=true;
    }
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
    gui->render();

    reBuildCommandBuffers(); // rebuild all cmdbufs every frame!!

    VulkanBase::prepareFrame();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VulkanBase::submitFrame();
  }

  virtual void loop(bool focused)
  {
    if(pendingAlarmUID){
      onex_run_evaluators(pendingAlarmUID, 0);
      free(pendingAlarmUID);
      pendingAlarmUID=0;
    }
    onex_loop(); // !focused=sleeping
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

extern void set_blemac(char*);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
extern "C" {

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jobject thiz, jint keyCode, jint u32key)
{
  static_gui->keyPressed(keyCode, u32key);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyRelease(JNIEnv* env, jobject thiz, jint keyCode)
{
  static_gui->keyReleased(keyCode);
}

extern void serial_on_recv(char*, int);

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_serialOnRecv(JNIEnv* env, jobject thiz, jstring b)
{
  const char* chars = b? env->GetStringUTFChars(b, 0): 0;
  serial_on_recv((char*)chars, chars? strlen(chars): 0);
  if(b) env->ReleaseStringUTFChars(b, chars);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onAlarmRecv(JNIEnv* env, jobject thiz, jstring juid)
{
  const char* uid = env->GetStringUTFChars(juid, 0);
  log_write("onAlarmRecv=%s\n",uid);
  pendingAlarmUID=strdup(uid);
  env->ReleaseStringUTFChars(juid, uid);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_setBLEMac(JNIEnv* env, jobject thiz, jstring jblemac)
{
  char* blemac = (char*)env->GetStringUTFChars(jblemac, 0);
  set_blemac(blemac);
  env->ReleaseStringUTFChars(jblemac, blemac);
}

}
#endif
