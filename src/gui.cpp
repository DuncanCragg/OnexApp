/*
* Vulkan Example - imGui (https://github.com/ocornut/imgui)
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "gui.h"

  GUI::GUI(VulkanBase *a)
  {
    app = a;
  };

  void GUI::prepare()
  {
    device = app->vulkanDevice;
    initImGUI((float)app->width, (float)app->height);
    getFontInfo();
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

  ImVec4 actionTextColour(0.5f, 0.1f, 0.2f, 1.0f);
  ImVec4 propertyColour(0.2f, 0.5f, 0.3f, 1.0f);
  ImVec4 keywordColour(0.5f, 0.1f, 0.2f, 1.0f);
  ImVec4 propertyBackground(0.90f, 0.90f, 1.00f, 1.00f);
  ImVec4 valueBackground(0.9f, 0.8f, 1.0f, 1.0f);

  void GUI::initImGUI(float width, float height)
  {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.9f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ComboBg] = valueBackground;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_Button] = valueBackground;
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.6f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = valueBackground;
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
//  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_Header] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_Column] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_CloseButton] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//  style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.IniFilename = 0;
    window_flags |= ImGuiWindowFlags_ShowBorders;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
    window_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    window_flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
//  window_flags |= ImGuiWindowFlags_NoTitleBar;
//  window_flags |= ImGuiWindowFlags_NoScrollbar;
//  window_flags |= ImGuiWindowFlags_NoCollapse;
//  window_flags |= ImGuiWindowFlags_MenuBar;
//  window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
//  window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//  window_flags |= ImGuiWindowFlags_NoSavedSettings;
//  window_flags |= ImGuiWindowFlags_NoInputs;
//  window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
//  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
//  window_flags |= ImGuiWindowFlags_AlwaysUseWindowPadding;
  }

#if defined(__ANDROID__)
#define ASSET_PATH ""
#else
#define ASSET_PATH "./../data/"
#endif

#if defined(__ANDROID__)
  bool keyboardUp = false;

  void GUI::showOrHideSoftKeyboard(bool show)
  {
    if(keyboardUp == show) return;
    JNIEnv* jni;
    androidApp->activity->vm->AttachCurrentThread(&jni, 0);
    jobject nativeActivity = androidApp->activity->clazz;
    jclass nativeActivityClass = jni->GetObjectClass(nativeActivity);
    jmethodID method = jni->GetMethodID(nativeActivityClass, show? "showKeyboard": "hideKeyboard", "()V");
    jni->CallVoidMethod(nativeActivity, method);
    androidApp->activity->vm->DetachCurrentThread();
    keyboardUp = show;
  }
#endif

  void GUI::getFontInfo()
  {
    ImGuiIO& io = ImGui::GetIO();
    const char* fontfile = ASSET_PATH "fonts/OpenSans-Regular.ttf";
    float fontsize = 60.0f;
#if defined(__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, fontfile, AASSET_MODE_STREAMING);
    size_t size = AAsset_getLength(asset);
    char* font_data = new char[size]; // TODO free?
    AAsset_read(asset, font_data, size);
    AAsset_close(asset);
    io.Fonts->AddFontFromMemoryTTF(font_data, size, fontsize, NULL, io.Fonts->GetGlyphRangesDefault());
#else
    io.Fonts->AddFontFromFileTTF(fontfile, fontsize, NULL, io.Fonts->GetGlyphRangesDefault());
#endif
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
  }

  struct UISettings {
    bool banana = false;
    float bananaSpeed = 0.25f;
  } uiSettings;

  enum {
    INITIAL = 0,
    PROPERTY_CHOOSER,
    ENTER_PROPERTY_NAME,
    ENTER_PROPERTY_VAL,
    SHOW_OBJECT,
    XXX,
    FINAL
  } state = INITIAL;

  char* propname;

  properties* drawThis = 0;

  void GUI::drawProperties(properties* p)
  {
    drawThis = p;
  }

  void GUI::drawGUI()
  {
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2((float)app->width, (float)app->height), ImGuiSetCond_FirstUseEver);
    if (!ImGui::Begin("Onex App Maker", NULL, window_flags))
    {
        ImGui::End();
        ImGui::Render();
        return;
    }

    int svs = 5;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10,5));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f,0));

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScrollbarSize = 40.0f;

    if(state == INITIAL) drawInitial();
    if(state == PROPERTY_CHOOSER) drawInitial();
    if(state == ENTER_PROPERTY_NAME) drawInitial();
    if(state == ENTER_PROPERTY_VAL) drawInitial();
    if(state == SHOW_OBJECT) drawInitial();
    if(state == XXX) drawXXX();
    if(state == FINAL)   drawFinal();

    ImGui::PopStyleVar(svs);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(650, 650), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow();

    ImGui::Render();
  }

  static int propchoice = 0;

  object* newObject = 0;

  void GUI::drawObjectProperties(properties* p, bool locallyEditable)
  {
    for(int i=1; i<= properties_size(p); i++){
      if(!locallyEditable && i==properties_size(p)) ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,100));
      drawProperty(properties_get_key(p, i), properties_get_val(p, i));
      if(!locallyEditable && i==properties_size(p)) ImGui::PopStyleVar();
    }
    if(locallyEditable){
      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      if(state==PROPERTY_CHOOSER){
        ImGui::PushItemWidth(280); ImGui::Combo("", &propchoice, "choose..\0new\0title\0description\0Rules\0Notifying\0Alerted\0Timer\0"); ImGui::PopItemWidth();
        if(ImGui::IsItemHovered()) ImGui::SetTooltip("Choose or enter the name of a property here");
        if(propchoice == 1){ state = ENTER_PROPERTY_NAME; propchoice = 0; }
        if(propchoice == 4){ state = XXX; propchoice = 0; }
      }
      if(state==ENTER_PROPERTY_NAME){
        static char b[64] = "";
        struct TextFilters {
          static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) {
            ImWchar ch = data->EventChar;
            if(ch >=256) return 1;
            if(!strlen(b) && !isalpha(ch)) return 1;
            if(ch == ' '){ data->EventChar = '-'; return 0; }
            if(ch == '-'){ return 0; }
            if(!isalnum(ch)) return 1;
            data->EventChar = tolower(ch);
            return 0;
          }
        };
        ImGui::SetKeyboardFocusHere();
#if defined(__ANDROID__)
        showOrHideSoftKeyboard(true);
#endif
        ImGui::PushItemWidth(280);
        if(ImGui::InputText("## property name", b, 64, ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue, TextFilters::FilterImGuiLetters)){
          object_property_set(newObject, strdup(b), (char*)"");
          state=SHOW_OBJECT;
          *b=0;
        }
        ImGui::PopItemWidth();
      }
      if(state==SHOW_OBJECT){
        if(ImGui::Button(" ", ImVec2(280, 70))) state=PROPERTY_CHOOSER;
      }
      ImGui::PopStyleColor(2);

      if(state!=ENTER_PROPERTY_VAL){
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,100));
        ImGui::Button("", ImVec2(610, 70));
        ImGui::PopStyleVar();
      }
    }
  }

  void GUI::drawProperty(char* key, char* val)
  {
    uint16_t height = 70;
    ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
    ImGui::Button(key, ImVec2(280, height));
    ImGui::PopStyleColor(2);
    ImGui::SameLine();
    if(state!=ENTER_PROPERTY_VAL || strcmp(key, propname)){
      if(ImGui::Button(val, ImVec2(610, height))){ state = ENTER_PROPERTY_VAL; propname = key; }
    }
    else{
      static char b[64] = "";
      struct TextFilters {
        static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) {
          ImWchar ch = data->EventChar;
          if(ch >=256) return 1;
          return 0;
        }
      };
      strncpy(b, val, 64); b[63]=0;
      ImGui::SetKeyboardFocusHere();
#if defined(__ANDROID__)
      showOrHideSoftKeyboard(true);
#endif
      ImGui::PushItemWidth(610);
      if(ImGui::InputText(" ", b, 64, ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue, TextFilters::FilterImGuiLetters)){
        object_property_set(newObject, propname, strdup(b));
        state=SHOW_OBJECT;
        *b=0;
      }
      ImGui::PopItemWidth();
    }
  }

  static bool evaluate_any_object(object* user)
  {
    return true;
  }

  void GUI::drawInitial()
  {
    ImGui::BeginChild("Workspace1", ImVec2(910,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100,100));
      ImGui::PushStyleColor(ImGuiCol_Text, actionTextColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      if(ImGui::Button("Add new object", ImVec2(380, 70))){
        state = PROPERTY_CHOOSER;
        if(!newObject) newObject = object_new((char*)"uid-2", (char*)"editable", evaluate_any_object, 4);
      }
      if(ImGui::IsItemHovered()) ImGui::SetTooltip("Start here!");
      ImGui::PopStyleColor(2);
      ImGui::PopStyleVar();

      if(drawThis) drawObjectProperties(drawThis, false);

      if(newObject){
        properties* p = object_properties(newObject, (char*)":");
        drawObjectProperties(p, true);
      }
    }
    ImGui::EndChild();
  }

  void GUI::drawXXX()
  {
    ImGui::BeginChild("Workspace1", ImVec2(910,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100,100));

      ImGui::PushStyleColor(ImGuiCol_Text, actionTextColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      if(ImGui::Button("Add new object", ImVec2(380, 70))) state = PROPERTY_CHOOSER;
      ImGui::PopStyleColor(2);

      ImGui::PopStyleVar();

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      if(ImGui::Button("Rules", ImVec2(280, 70))) state=FINAL;
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      ImGui::Button("", ImVec2(610, 70));

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("new-todo", ImVec2(280, 70));
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      ImGui::Button("What needs doing?", ImVec2(610, 70));

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("todos", ImVec2(280, 200));
      ImGui::PopStyleColor(2);

      ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, valueBackground);
      ImGui::SameLine();
      ImGui::BeginChild("Todos", ImVec2(0,200), true, ImGuiWindowFlags_HorizontalScrollbar);
      {
        ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
        ImGui::Button("title", ImVec2(280, 70));
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::Button("milk", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("beer", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("potatoes", ImVec2(280, 70));

        ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
        ImGui::Button("done", ImVec2(280, 70));
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::Button("yes", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("no", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("no", ImVec2(280, 70));
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();
    }
    ImGui::EndChild();
  }

  void GUI::drawFinal()
  {
    ImGui::BeginChild("Workspace1", ImVec2(910,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100,100));

      ImGui::PushStyleColor(ImGuiCol_Text, actionTextColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      if(ImGui::Button("Add new object", ImVec2(380, 70))) state = PROPERTY_CHOOSER;
      ImGui::PopStyleColor(2);

      ImGui::PopStyleVar();

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("Rules", ImVec2(280, 70));
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      ImGui::Button("", ImVec2(610, 70));

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("new-todo", ImVec2(280, 70));
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      ImGui::Button("What needs doing?", ImVec2(610, 70));

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("todos", ImVec2(280, 200));
      ImGui::PopStyleColor(2);

      ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, valueBackground);
      ImGui::SameLine();
      ImGui::BeginChild("Todos", ImVec2(0,200), true, ImGuiWindowFlags_HorizontalScrollbar);
      {
        ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
        ImGui::Button("title", ImVec2(280, 70));
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::Button("milk", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("beer", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("potatoes", ImVec2(280, 70));

        ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
        ImGui::Button("done", ImVec2(280, 70));
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::Button("yes", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("no", ImVec2(280, 70));
        ImGui::SameLine();
        ImGui::Button("no", ImVec2(280, 70));
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(-1,0));
    ImGui::SameLine();
    ImGui::PopStyleVar();
    ImGui::BeginChild("Workspace2", ImVec2(2000,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("", ImVec2(280, 70));
      ImGui::PopStyleColor(2);
      ImGui::SameLine();
      ImGui::Button("", ImVec2(600, 70));
      ImGui::SameLine();
      ImGui::Button("", ImVec2(60, 70));
      ImGui::SameLine();
      ImGui::Button("", ImVec2(1040, 70));

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("todos", ImVec2(280, 200));
      ImGui::PopStyleColor(2);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, keywordColour);
      ImGui::Button("list", ImVec2(600, 200));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, keywordColour);
      ImGui::Button("->", ImVec2(60, 200));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::Button("todos", ImVec2(140, 200));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, keywordColour);
      ImGui::Button("joined\nwith", ImVec2(150, 200));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, valueBackground);
      ImGui::BeginChild("newtodo", ImVec2(0,200), true, ImGuiWindowFlags_HorizontalScrollbar);
      {
        ImGui::BeginGroup();
        {
            ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
            ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
            ImGui::Button("title", ImVec2(280, 70));
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
            ImGui::Button("new-todo", ImVec2(280, 70));
            ImGui::PopStyleColor(1);

            ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
            ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
            ImGui::Button("done", ImVec2(280, 70));
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            ImGui::Button("no", ImVec2(280, 70));
        }
        ImGui::EndGroup();
      }
      ImGui::EndChild();
      ImGui::PopStyleColor(1);

      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::Button("new-todo", ImVec2(280, 70));
      ImGui::PopStyleColor(2);
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, keywordColour);
      ImGui::Button("not", ImVec2(100, 70));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::Button("What needs doing?", ImVec2(500, 70));
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, keywordColour);
      ImGui::Button("->", ImVec2(60, 70));
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      ImGui::Button("What needs doing?", ImVec2(1040, 70));
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::Text(" ");
  }

  void GUI::render()
  {
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)app->width, (float)app->height);
    io.DeltaTime = app->frameTimer;
    io.FontGlobalScale = 1.0;

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
#define LOG_UNICODE_BYTES
#ifdef LOG_UNICODE_BYTES
      uint8_t* u32b = (uint8_t*)&u32keyToAdd; log_write("bytes: %x %x %x %x\n", u32b[3], u32b[2], u32b[1], u32b[0]);
#endif
      u32keyToAdd = 0;
    }
  }

  void GUI::keyPressed(uint32_t keyCode, char32_t u32key)
  {
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
    ImGuiIO& io = ImGui::GetIO();
    if(keyCode) io.KeysDown[keyCode] = false;
    io.KeyCtrl = io.KeysDown[KEY_CTRL_LEFT] || io.KeysDown[KEY_CTRL_RIGHT];
    io.KeyShift = io.KeysDown[KEY_SHIFT_LEFT] || io.KeysDown[KEY_SHIFT_RIGHT];
    io.KeyAlt = io.KeysDown[KEY_ALT_LEFT] || io.KeysDown[KEY_ALT_RIGHT];
    io.KeySuper = io.KeysDown[KEY_SUPER_LEFT] || io.KeysDown[KEY_SUPER_RIGHT];
  }

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
      drawGUI();
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
