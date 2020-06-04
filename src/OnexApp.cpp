
#include "gui.h"
#include <pthread.h>

#define ENABLE_VALIDATION false

static GUI* static_gui;

bool keyboardUp = false;

#define TEXTTYPE 1

extern char* init_onex();
extern void  loop_onex();
extern void  set_blemac(char*);

char* pendingAlarmUID=0;

extern "C" {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)

extern void serial_on_recv(char*, int);

static jclass eternalServiceClass;

static JavaVM* javaVM;

static bool maybeAttachCurrentThreadAndFetchEnv(JNIEnv** env)
{
  jint res = javaVM->GetEnv((void**)env, JNI_VERSION_1_6);
  if(res==JNI_OK) return false;

  res=javaVM->AttachCurrentThread(env, 0);
  if(res==JNI_OK) return true;

  log_write("Failed to AttachCurrentThread, ErrorCode: %d", res);
  *env=0;
  return false;
}

// or see https://stackoverflow.com/questions/13263340/findclass-from-any-thread-in-android-jni
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
  javaVM=vm;

  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return 0;

  eternalServiceClass = (jclass)env->NewGlobalRef(env->FindClass("network/object/onexapp/EternalService"));

  if(attached) javaVM->DetachCurrentThread();

  return JNI_VERSION_1_6;
}

void sprintExternalStorageDirectory(char* buf, int buflen, const char* format)
{
  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return;

  jclass osEnvClass = env->FindClass("android/os/Environment");
  jmethodID getExternalStorageDirectoryMethod = env->GetStaticMethodID(osEnvClass, "getExternalStorageDirectory", "()Ljava/io/File;");
  jobject extStorage = env->CallStaticObjectMethod(osEnvClass, getExternalStorageDirectoryMethod);

  jclass extStorageClass = env->GetObjectClass(extStorage);
  jmethodID getAbsolutePathMethod = env->GetMethodID(extStorageClass, "getAbsolutePath", "()Ljava/lang/String;");
  jstring extStoragePath = (jstring)env->CallObjectMethod(extStorage, getAbsolutePathMethod);

  const char* extStoragePathString=env->GetStringUTFChars(extStoragePath, 0);
  snprintf(buf, buflen, format, extStoragePathString);
  env->ReleaseStringUTFChars(extStoragePath, extStoragePathString);

  if(attached) javaVM->DetachCurrentThread();
}

void serial_send(char* buff)
{
  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return;

  jmethodID method = env->GetStaticMethodID(eternalServiceClass, "serialSend", "(Ljava/lang/String;)V");
  jstring jbuff = env->NewStringUTF(buff);
  env->CallStaticVoidMethod(eternalServiceClass, method, jbuff);
  env->DeleteLocalRef(jbuff);

  if(attached) javaVM->DetachCurrentThread();
}

JNIEXPORT jstring JNICALL Java_network_object_onexapp_EternalService_initOnex(JNIEnv* env, jclass clazz)
{
  char* blemac=init_onex();
  jstring jblemac = env->NewStringUTF(blemac);
  return jblemac;
}

JNIEXPORT void JNICALL Java_network_object_onexapp_EternalService_loopOnex(JNIEnv* env, jclass clazz)
{
  loop_onex();
}

JNIEXPORT void JNICALL Java_network_object_onexapp_EternalService_setBLEMac(JNIEnv* env, jclass clazz, jstring jblemac)
{
  char* blemac = (char*)env->GetStringUTFChars(jblemac, 0);
  set_blemac(blemac);
  env->ReleaseStringUTFChars(jblemac, blemac);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_EternalService_serialOnRecv(JNIEnv* env, jclass clazz, jstring b)
{
  const char* chars = b? env->GetStringUTFChars(b, 0): 0;
  serial_on_recv((char*)chars, chars? strlen(chars): 0);
  if(b) env->ReleaseStringUTFChars(b, chars);
}

// ----------------------------- Activity

void showOrHideSoftKeyboard(bool show)
{
  if(keyboardUp == show) return;

  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return;

  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  if(show){
    jmethodID method = env->GetMethodID(nativeActivityClass, "showKeyboard", "(I)V");
    env->CallVoidMethod(nativeActivity, method, TEXTTYPE);
  }else{
    jmethodID method = env->GetMethodID(nativeActivityClass, "hideKeyboard", "()V");
    env->CallVoidMethod(nativeActivity, method);
  }
  keyboardUp = show;

  if(attached) javaVM->DetachCurrentThread();
}

void showNotification(char* title, char* text)
{
  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return;

  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "showNotification", "(Ljava/lang/String;Ljava/lang/String;)V");
  jstring jtitle = env->NewStringUTF(title);
  jstring jtext  = env->NewStringUTF(text);
  env->CallVoidMethod(nativeActivity, method, jtitle, jtext);
  env->DeleteLocalRef(jtitle); env->DeleteLocalRef(jtext);

  if(attached) javaVM->DetachCurrentThread();
}

void setAlarm(time_t when, char* uid)
{
  log_write("setAlarm %ld %s\n", when, uid);

  JNIEnv* env;
  bool attached=maybeAttachCurrentThreadAndFetchEnv(&env);
  if(!env) return;

  jobject nativeActivity = androidApp->activity->clazz;
  jclass nativeActivityClass = env->GetObjectClass(nativeActivity);
  jmethodID method = env->GetMethodID(nativeActivityClass, "setAlarm", "(JLjava/lang/String;)V");
  jlong jwhen = (jlong)when;
  jstring juid  = env->NewStringUTF(uid);
  env->CallVoidMethod(nativeActivity, method, jwhen, juid);
  env->DeleteLocalRef(juid);

  if(attached) javaVM->DetachCurrentThread();
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyPress(JNIEnv* env, jclass clazz, jint keyCode, jint u32key)
{
  static_gui->keyPressed(keyCode, u32key);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onKeyRelease(JNIEnv* env, jclass clazz, jint keyCode)
{
  static_gui->keyReleased(keyCode);
}

JNIEXPORT void JNICALL Java_network_object_onexapp_OnexNativeActivity_onAlarmRecv(JNIEnv* env, jclass clazz, jstring juid)
{
  const char* uid = env->GetStringUTFChars(juid, 0);
  log_write("onAlarmRecv=%s\n",uid);
  pendingAlarmUID=strdup(uid);
  env->ReleaseStringUTFChars(juid, uid);
}

#else
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

static pthread_t thread_id;

static void* loop_onex_thread(void* data)
{
  loop_onex();
  return 0;
}

bool front_end_running=false;

class OnexApp : public VulkanBase
{
  GUI* gui;

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
#if defined(VK_USE_PLATFORM_XCB_KHR)
    init_onex();
    pthread_create(&thread_id, 0, loop_onex_thread, 0);
#endif
    log_write("OnexApp----------------------\n");
    VulkanBase::prepare();
    gui->prepare();
    prepared = true;
    front_end_running=true;
    if(keyboardUp){ keyboardUp = false; showOrHideSoftKeyboard(true); }
    log_write("-----------------------------\n");
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

  virtual void keyPressed(int32_t keyCode, char32_t u32key)
  {
    gui->keyPressed(keyCode, u32key);
  }

  virtual void keyReleased(int32_t keyCode)
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
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#define VULKAN_EXAMPLE_MAIN()
#endif
//VULKAN_EXAMPLE_MAIN()

OnexApp *vulkanApp;

#if defined(VK_USE_PLATFORM_XCB_KHR)

static void handleEvent(const xcb_generic_event_t *event)
{
  if (vulkanApp != NULL)
  {
    vulkanApp->handleEvent(event);
  }
}

int main(const int argc, const char *argv[])
{
  for (size_t i = 0; i < argc; i++) { OnexApp::args.push_back(argv[i]); };
  vulkanApp = new OnexApp();
  vulkanApp->initVulkan();
  vulkanApp->setupWindow();
  vulkanApp->initSwapchain();
  vulkanApp->prepare();
  vulkanApp->renderLoop();
  delete(vulkanApp);
  return 0;
}

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)

void android_main(android_app* state)
{
  androidApp = state;
  vulkanApp = new OnexApp();
  state->userData = vulkanApp;
  state->onAppCmd = OnexApp::handleAppCommand;
  state->onInputEvent = OnexApp::handleAppInput;
  vks::android::getDeviceConfig();
  vulkanApp->renderLoop();
  delete(vulkanApp);
}

#endif

