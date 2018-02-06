/*
* GUI using Vulkan for OnexApp
*
* Based on Sascha Willems' imGui Vulkan Example
*
* Those parts Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "gui.h"

GUI::GUI(VulkanBase* a, object* u)
{
  app = a;
  user = u;
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

// ---------------------------------------------------------------------------------------------

ImVec4 actionColour            (0.50f, 0.10f, 0.20f, 1.0f);
ImVec4 actionBackground        (0.96f, 0.96f, 0.87f, 1.0f);
ImVec4 actionBackgroundActive  (0.92f, 0.92f, 0.83f, 1.0f);

ImVec4 propertyColour          (0.20f, 0.50f, 0.30f, 1.0f);
ImVec4 propertyBackground      (0.90f, 0.90f, 1.00f, 1.0f);
ImVec4 propertyBackgroundActive(0.86f, 0.86f, 0.95f, 1.0f);

ImVec4 valueBackground         (0.96f, 0.87f, 1.00f, 1.0f);
ImVec4 valueBackgroundActive   (0.92f, 0.82f, 0.96f, 1.0f);

ImVec4 listBackground          (0.90f, 0.80f, 1.00f, 1.0f);
ImVec4 listBackgroundDark      (0.86f, 0.76f, 0.96f, 1.0f);

ImVec4 schemeBrown(183.0f/255, 142.0f/255, 96.0f/255, 1.0f);
ImVec4 schemeYellow(255.0f/255, 245.0f/255, 180.0f/255, 1.0f);
ImVec4 schemeMauve(221.0f/255, 190.0f/255, 243.0f/255, 1.0f);
ImVec4 schemePurple(169.0f/255, 103.0f/255, 212.0f/255, 1.0f);
ImVec4 schemeGreen(160.0f/255, 175.0f/255, 110.0f/255, 1.0f);
ImVec4 schemeLightPurple(0.8f, 0.7f, 0.9f, 1.0f);
ImVec4 schemeDarkerPurple(0.73f, 0.63f, 0.83f, 1.0f);
ImVec4 schemePlum(230.0f/255, 179.0f/255, 230.0f/255, 1.0f);

#define shorterValWidth 680
#define objectHeight 400
#define listHeight 1000
#define buttonWidth 190
#define smallButtonWidth 90
#define buttonHeight 70
#define rhsPadding 20
#define paddingHeight 15

static uint16_t workspace1Width;
static uint16_t workspace1Height;

void GUI::initImGUI(float width, float height)
{
  workspace1Width=((int)width)/2-10;
  workspace1Height=(int)height-10;
  ImGuiStyle& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_Border] = propertyBackgroundActive;
  style.Colors[ImGuiCol_BorderShadow] = propertyBackgroundActive;
  style.Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.9f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 0.7f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarBg] = propertyBackground;
  style.Colors[ImGuiCol_ScrollbarGrab] = valueBackground;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = valueBackground;
  style.Colors[ImGuiCol_ScrollbarGrabActive] = valueBackgroundActive;
  style.Colors[ImGuiCol_PopupBg] = actionBackground;
  style.Colors[ImGuiCol_FrameBg] = actionBackground;
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_Button] = valueBackground;
  style.Colors[ImGuiCol_ButtonHovered] = valueBackground;
  style.Colors[ImGuiCol_ButtonActive] = valueBackgroundActive;
  style.Colors[ImGuiCol_SliderGrab] = valueBackground;
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_Header] = actionBackground;
  style.Colors[ImGuiCol_HeaderHovered] = actionBackground;
  style.Colors[ImGuiCol_HeaderActive] = actionBackgroundActive;
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
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
//window_flags |= ImGuiWindowFlags_NoResize;
//window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
//window_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
//window_flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
//  window_flags |= ImGuiWindowFlags_NoTitleBar;
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

// ---------------------------------------------------------------------------------------------

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

// Use ImGui::ShowStyleEditor() to look them up.

  int svs = 7;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10,5));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

  ImGuiStyle& style = ImGui::GetStyle();
  style.ScrollbarSize = 0.0f;
  style.TouchExtraPadding = ImVec2(10.0f,10.0f);

  drawView();

  ImGui::PopStyleVar(svs);

  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(650, 650), ImGuiSetCond_FirstUseEver);
  ImGui::ShowTestWindow();

  ImGui::Render();
}

const char* propNameStrings[] = { "+", "+value(s)..", "+object(s)..", "title", "description", "text", "date", "start-date", "end-date", "list", "Rules", "Timer" };
int         propNameChoice = 0;
char*       propNameEditing=0;
uint16_t    yOffsetTarget=0;
uint16_t    yOffset=0;
uint16_t    yOffsetCounter=0;

void GUI::showKeyboard(float multy){
#if defined(__ANDROID__)
  if(yOffsetTarget) return;
  yOffsetTarget=(multy!=0)? multy: (ImGui::GetCursorScreenPos().y-buttonHeight)/1.85;
  yOffsetCounter=100;
  showOrHideSoftKeyboard(true);
#endif
}

void GUI::hideKeyboard(){
#if defined(__ANDROID__)
  yOffsetTarget=0;
  yOffsetCounter=0;
  yOffset=0;
  showOrHideSoftKeyboard(false);
#endif
}

void GUI::drawView()
{
  ImGui::BeginChild("Workspace1", ImVec2(workspace1Width,0), true);
  {
    char* uid=object_property(user, (char*)"viewing");
    bool locallyEditable = object_is_local(uid);
    char path[9]; memcpy(path, "viewing:", 9);
#if defined(__ANDROID__)
    ImVec2 startingpoint = ImGui::GetCursorScreenPos();
    if(yOffsetCounter){
      yOffset=yOffsetTarget*(100-yOffsetCounter)/100;
      yOffsetCounter-=5;
    }
    ImVec2 startpos(startingpoint.x, startingpoint.y - yOffset);
    ImGui::SetCursorScreenPos(startpos);
#endif
    if(user) drawObjectProperties(path, locallyEditable, workspace1Width-rhsPadding, workspace1Height, 1);
  }
  ImGui::EndChild();
}

static bool evaluate_any_object(object* user)
{
  return true;
}

static char* dragPathId=0;
static float delta_x=0.0f;
static float delta_y=0.0f;
static bool drag_handled = true;
#define MOVING_DELTA(x,y) (((x)*(x)+(y)*(y)) > 0.01f)

static void track_drag(char* pathId)
{
  if(ImGui::IsItemActive() && !ImGui::IsMouseDragging() && dragPathId && strcmp(pathId, dragPathId)){
    free(dragPathId);
    dragPathId=0;
    delta_x = 0.0f;
    delta_y = 0.0f;
    drag_handled=true;
  }
  else
  if(ImGui::IsItemActive() && ImGui::IsMouseDragging()){
    if(!dragPathId || strcmp(dragPathId, pathId)) dragPathId=strdup(pathId);
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    if(MOVING_DELTA(mouse_delta.x, mouse_delta.y)){
      delta_x=mouse_delta.x;
      delta_y=mouse_delta.y;
      drag_handled=false;
    }
  }
  else
  if(MOVING_DELTA(delta_x, delta_y) && dragPathId && !strcmp(pathId, dragPathId)){
    delta_x *= 0.99f;
    delta_y *= 0.99f;
    drag_handled=false;
  }
  else
  if(!ImGui::IsMouseDragging() && dragPathId && !strcmp(pathId, dragPathId)){
    free(dragPathId);
    dragPathId=0;
  }
}

static void set_drag_scroll(char* path)
{
  if(!dragPathId) return;
  char* dragPathPath=strstr(dragPathId, "viewing:");
  if(!strncmp(dragPathPath, path, strlen(path)) && strcmp(dragPathPath, path) && !drag_handled && MOVING_DELTA(delta_x,delta_y)){
    ImGui::SetScrollX(ImGui::GetScrollX() - delta_x);
    ImGui::SetScrollY(ImGui::GetScrollY() - delta_y);
    drag_handled=true;
  }
}

void GUI::setNewValue(char* path, char* valBuf, bool single)
{
  char* lastcolon=strrchr(path,':'); *lastcolon=0;
  if(single){
    char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
    object* objectEditing = onex_get_from_cache(object_property(user, path));
    if(!*valBuf) object_property_set(objectEditing, secondlastcolon+1, (char*)"");
    else object_property_set(objectEditing, secondlastcolon+1, strdup(valBuf));
    *secondlastcolon=':';
  }
  else{
    char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
    char* thirdlastcolon=strrchr(path, ':'); *thirdlastcolon=0;
    object* objectEditing = onex_get_from_cache(object_property(user, path));
    *secondlastcolon=':';
    if(!*valBuf) object_property_set(objectEditing, thirdlastcolon+1, (char*)"");
    else object_property_set(objectEditing, thirdlastcolon+1, strdup(valBuf));
    *thirdlastcolon=':';
  }
  *lastcolon=':';
}

void GUI::setPropertyName(char* path , char* name)
{
  char* lastcolon=strrchr(path,':');
  *lastcolon=0;
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  *lastcolon=':';
  object_property_set(objectEditing, name, (char*)"--");
}

void GUI::setPropertyNameAndObject(char* path , char* name)
{
  char* lastcolon=strrchr(path,':');
  *lastcolon=0;
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  *lastcolon=':';
  object* o = createNewObjectForPropertyName(path, name);
  if(o) object_property_set(objectEditing, name, object_property(o, (char*)"UID"));
  else object_property_set(objectEditing, name, (char*)"---");
}

void GUI::drawNewPropertyValueEditor(char* path, char* val, bool single, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
{
  if(!val){ log_write("val==null: path=%s\n", path); return; }
  char valId[256]; snprintf(valId, 256, "## val %s %s", val, path);
  static char valBuf[256] = ""; strncpy(valBuf, val, 256); valBuf[255]=0;
  ImGuiIO& io = ImGui::GetIO();
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(editing && !io.WantTextInput){
    hideKeyboard();
    free(propNameEditing); propNameEditing=0;
    *valBuf=0;
    editing=false;
  }
  ImGui::PushItemWidth(width);
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_FrameBg, nodarken? valueBackground: valueBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, valueBackgroundActive);
  int flags=ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine|ImGuiInputTextFlags_AutoSelectAll;
  if(!editing){
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, valueBackground);
    float multy=ImGui::GetCursorScreenPos().y-buttonHeight;
    if(height==buttonHeight){ ImGui::InputText(valId, valBuf, 256, flags); multy=0; }
    else                      ImGui::InputTextMultiline(valId, valBuf, 256, ImVec2(-1.0f, height-40), flags);
    if(ImGui::IsItemActive() && ImGui::IsMouseReleased(0) && !dragPathId){
      if(locallyEditable){
        propNameEditing = strdup(path);
        showKeyboard(multy);
      }
    }
    track_drag(valId);
    ImGui::PopStyleColor();
  }
  else{
    bool done=false;
    if(height==buttonHeight) done=ImGui::InputText(valId, valBuf, 256, flags);
    else                     done=ImGui::InputTextMultiline(valId, valBuf, 256, ImVec2(-1.0f, height-40), flags);
    if(done){
      setNewValue(path, valBuf, single);
      hideKeyboard();
      free(propNameEditing); propNameEditing=0;
      *valBuf=0;
    }
  }
  ImGui::PopStyleColor(2);
  ImGui::PopItemWidth();
}

void GUI::drawObjectFooter(char* path, bool locallyEditable, int16_t width, int16_t keyWidth, int8_t depth)
{
  if(depth>=3) return;
  if(!locallyEditable) return;
  static bool grabbedFocus=false;
  static char valBuf[256] = "";
  ImGuiIO& io = ImGui::GetIO();
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(editing && grabbedFocus && !io.WantTextInput){
    hideKeyboard();
    free(propNameEditing); propNameEditing=0; propNameChoice = 0;
    *valBuf=0;
    grabbedFocus=false;
    editing=false;
  }
  if(!editing){
    ImGui::PushItemWidth(keyWidth);
    int c=0;
    char comId[256]; snprintf(comId, 256, "## combo %s", path);
    bool nodarken=depth<3;
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 50);
    ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, nodarken? propertyBackground: propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, nodarken? propertyBackground: propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_Button, nodarken? propertyBackground: propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? propertyBackground: propertyBackgroundActive);
    ImGui::Combo(comId, !propNameEditing? &propNameChoice: &c, propNameStrings, IM_ARRAYSIZE(propNameStrings));
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar();
    track_drag(comId);
    if(!propNameEditing && propNameChoice){ propNameEditing = strdup(path); if(propNameChoice==1 || propNameChoice==2) showKeyboard(0); }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    int blankwidth = width - keyWidth;
    if(blankwidth>10){
      char barId[256]; snprintf(barId, 256, "## comboblank %s", path);
      ImGui::PushStyleColor(ImGuiCol_Button, nodarken? valueBackground: valueBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? valueBackground: valueBackgroundActive);
      ImGui::Button(barId, ImVec2(blankwidth, buttonHeight));
      ImGui::PopStyleColor(2);
      track_drag(barId);
    }
  }else{
    int flags=ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue;
    if(propNameChoice > 2){
      char* propname=(char*)propNameStrings[propNameChoice];
      if(!strcmp(propname, "list") || !strcmp(propname, "Rules")) setPropertyNameAndObject(path, propname);
      else setPropertyName(path, propname);
      free(propNameEditing); propNameEditing=0; propNameChoice = 0;
    }
    else if(propNameChoice==1){
      struct TextFilters {
        static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) {
          ImWchar ch = data->EventChar;
          if(ch >=256) return 1;
          if(!strlen(valBuf) && !isalpha(ch)) return 1;
          if(ch == ' '){ data->EventChar = '-'; return 0; }
          if(ch == '-'){ return 0; }
          if(!isalnum(ch)) return 1;
          data->EventChar = tolower(ch);
          return 0;
        }
      };
      if(!grabbedFocus){
        ImGui::SetKeyboardFocusHere();
        grabbedFocus = io.WantTextInput;
      }
      ImGui::PushItemWidth(keyWidth);
      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_PopupBg, propertyBackground);
      ImGui::PushStyleColor(ImGuiCol_FrameBg, propertyBackground);
      if(ImGui::InputText("## property name", valBuf, 256, flags, TextFilters::FilterImGuiLetters)){
        setPropertyName(path, strdup(valBuf));
        hideKeyboard();
        free(propNameEditing); propNameEditing=0; propNameChoice = 0;
        *valBuf=0;
        grabbedFocus=false;
      }
      ImGui::PopStyleColor(3);
      ImGui::PopItemWidth();
      ImGui::SameLine();
      int blankwidth = width - keyWidth;
      if(blankwidth>10) ImGui::Button("--## blank", ImVec2(blankwidth, buttonHeight));
    }
    else if(propNameChoice==2){
      struct TextFilters {
        static int FilterImGuiLetters(ImGuiTextEditCallbackData* data) {
          ImWchar ch = data->EventChar;
          if(ch >=256) return 1;
          if(!strlen(valBuf) && !isalpha(ch)) return 1;
          if(ch == ' '){ data->EventChar = '-'; return 0; }
          if(ch == '-'){ return 0; }
          if(!isalnum(ch)) return 1;
          data->EventChar = tolower(ch);
          return 0;
        }
      };
      if(!grabbedFocus){
        ImGui::SetKeyboardFocusHere();
        grabbedFocus = io.WantTextInput;
      }
      ImGui::PushItemWidth(keyWidth);
      ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
      ImGui::PushStyleColor(ImGuiCol_PopupBg, propertyBackground);
      ImGui::PushStyleColor(ImGuiCol_FrameBg, propertyBackground);
      if(ImGui::InputText("## property name", valBuf, 256, flags, TextFilters::FilterImGuiLetters)){
        setPropertyNameAndObject(path, strdup(valBuf));
        hideKeyboard();
        free(propNameEditing); propNameEditing=0; propNameChoice = 0;
        *valBuf=0;
        grabbedFocus=false;
      }
      ImGui::PopStyleColor(3);
      ImGui::PopItemWidth();
      ImGui::SameLine();
      int blankwidth = width - keyWidth;
      if(blankwidth>10) ImGui::Button("[]## blank", ImVec2(blankwidth, buttonHeight));
    }
  }
}

void GUI::drawPadding(char* path, int16_t width, int16_t height, int8_t depth)
{
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_Border, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_BorderShadow, nodarken? listBackground: listBackgroundDark);
  char blnId[256]; snprintf(blnId, 256, "##padding %d %d %s", width, height, path);
  ImGui::Button(blnId, ImVec2(width, height));
  track_drag(blnId);
  ImGui::PopStyleColor(4);
}

void GUI::drawNewValueOrObjectButton(char* path, int16_t width, int j, int8_t depth)
{
  char pathj[256]; snprintf(pathj, 256, "%s:%d:", path, j);
  char addValId[256]; snprintf(addValId, 256, "+value ## %s", pathj);
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? valueBackground: valueBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? valueBackground: valueBackgroundActive);
  if(ImGui::Button(addValId, ImVec2(width/2, buttonHeight)) && !dragPathId){
    char* lastcolon=strrchr(path,':');
    char* secondlastcolon=0;
    char* vkey;
    if(lastcolon+1-path == strlen(path)){
      *lastcolon=0;
      secondlastcolon=strrchr(path,':');
      *secondlastcolon=0;
      vkey=strdup(secondlastcolon+1);
    }
    else{
      *lastcolon=0;
      vkey=strdup(lastcolon+1);
    }
    object* v = onex_get_from_cache(object_property(user, path));
    if(secondlastcolon) *secondlastcolon=':';
    *lastcolon=':';
    object_property_add(v, vkey, (char*)"--");
    free(vkey);
  }
  ImGui::PopStyleColor(2);
  track_drag(addValId);

  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, actionBackgroundActive);
  ImGui::SameLine();
  char addObjId[256]; snprintf(addObjId, 256, "+object ## %s", pathj);
  if(ImGui::Button(addObjId, ImVec2(width/2, buttonHeight)) && !dragPathId){
    char* lastcolon=strrchr(path,':');
    char* secondlastcolon=0;
    char* vkey;
    if(lastcolon+1-path == strlen(path)){
      *lastcolon=0;
      secondlastcolon=strrchr(path,':');
      *secondlastcolon=0;
      vkey=strdup(secondlastcolon+1);
    }
    else{
      *lastcolon=0;
      vkey=strdup(lastcolon+1);
    }
    object* v = onex_get_from_cache(object_property(user, path));
    if(secondlastcolon) *secondlastcolon=':';
    *lastcolon=':';
    object* o = createNewObjectLikeOthers(path);
    if(o) object_property_add(v, vkey, object_property(o, (char*)"UID"));
    free(vkey);
  }
  track_drag(addObjId);
  ImGui::PopStyleColor(4);
}

object* GUI::createNewObjectForPropertyName(char* path, char* name)
{
  object* r=object_new(0, 0, evaluate_any_object, 4);
  char* is;
  if(!strcmp(name,"Rules")) is=(char*)"Rule";
  else is=name;
  object_property_set(r, (char*)"is", strdup(is));
  return r;
}

object* GUI::createNewObjectLikeOthers(char* path)
{
  object* r=object_new(0, 0, evaluate_any_object, 4);
  bool filled=false;
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    if(key){
      char* is=0; if(!strcmp(key,"is")) is=object_property_value(user, path, i);
      object_property_set(r, key, is? is: (char*)"--");
      filled=true;
    }
    else{
      char* val=object_property_value(user, path, i);
      size_t l=strlen(path);
      snprintf(path+l, 128-l, ":%d:", i);
      size_t size=object_property_size(user, path);
      if(is_uid(val) && size){
        for(int j=1; j<=size; j++){
          char* key=object_property_key(user, path, j);
          if(key){
            char* is=0; if(!strcmp(key,"is")) is=object_property_value(user, path, j);
            object_property_set(r, key, is? is: (char*)"--");
            filled=true;
          }
        }
      }
      path[l] = 0;
    }
  }
  if(!filled) object_property_set(r, (char*)"is", (char*)"editable");
  return r;
}

static bool open[]={ true, true, true, false, false };

void GUI::drawObjectHeader(char* path, bool locallyEditable, int16_t width, int8_t depth)
{
  bool nodarken=depth<3;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, actionBackgroundActive);

  if(depth==1){
    char linkId[256]; snprintf(linkId, 256, " <<## %s", path);
    if(ImGui::Button(linkId, ImVec2(smallButtonWidth, buttonHeight))){
      uint16_t histlen=object_property_size(user, (char*)"history");
      char* viewing = object_property_value(user, (char*)"history", histlen);
      char popPath[64]; snprintf(popPath, 64, "history:%d:", histlen);
      object_property_set(user, (char*)popPath, 0);
      object_property_set(user, (char*)"viewing", viewing);
    }
    track_drag(linkId);
    ImGui::SameLine();
  }
  if(depth!=1 && depth<3){
    char dropId[256]; snprintf(dropId, 256, " X## %s", path);
    if(ImGui::Button(dropId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
      char* lastcolon=strrchr(path,':'); *lastcolon=0;
      char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
      char* thirdlastcolon=strrchr(path, ':'); *thirdlastcolon=0;
      object* objectEditing = onex_get_from_cache(object_property(user, path));
      *secondlastcolon=':';
      object_property_set(objectEditing, thirdlastcolon+1, (char*)"");
      *thirdlastcolon=':';
      *lastcolon=':';
    }
    track_drag(dropId);

    ImGui::SameLine();
  }

  int blankwidth = width-(depth<3? 2: 1)*smallButtonWidth-buttonHeight;
  if(blankwidth>10){
    char summary[128]="";
    getSummary(path, summary);
    char barId[256]; snprintf(barId, 256, "%s ## topbar %s", summary, path);
    ImGui::Button(barId, ImVec2(blankwidth, buttonHeight));
    track_drag(barId);
    ImGui::SameLine();
  }
  char expId[256]; snprintf(expId, 256, "## %s", path);
  ImGui::Checkbox(expId, &open[depth]);
  track_drag(expId);

  ImGui::SameLine();

  char maxId[256]; snprintf(maxId, 256, " [+]## %s", path);
  if(ImGui::Button(maxId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
    char* lastcolon=strrchr(path,':');
    *lastcolon=0;
    char* viewing=object_property(user, path);
    *lastcolon=':';
    object_property_add(user, (char*)"history", object_property(user, (char*)"viewing"));
    object_property_set(user, (char*)"viewing", viewing);
  }
  track_drag(maxId);

  ImGui::PopStyleColor(4);
  ImGui::PopStyleVar(1);
}

void GUI::getSummary(char* path, char* summary)
{
  uint8_t s=0;
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    if(strcmp(key, "title")) continue;
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s%s:", path, key);
    pathkey[l-1] = 0;
    if(object_property_is_value(user, pathkey)){
      pathkey[l-1] = ':';
      char* val=object_property_value(user, path, i);
      s+=snprintf(summary+s, 128-s, "%s ", val);
    }
    else
    if(object_property_is_list(user, pathkey)){
      uint8_t sz = object_property_size(user, pathkey);
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, pathkey, j);
        if(!is_uid(val)) s+=snprintf(summary+s, 128-s, "%s ", val);
      }
    }
  }
}

int16_t GUI::calculateKeyWidth(char* path)
{
  int16_t w=0;
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    if(key){
      int16_t l=strlen(key);
      if(l > w) w=l;
    }
  }
  return w>7? w*28: 196;
}

int16_t GUI::calculateScrollerHeight(char* path, int16_t height)
{
  int16_t heightforscrollers=height-3.5*buttonHeight;
  int8_t  numberofscrollers=0;
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s%s:", path, key);
    pathkey[l-1] = 0;
    if(object_property_is_value(user, pathkey)){
      pathkey[l-1] = ':';
      char* val=object_property_value(user, path, i);
      int16_t h = is_uid(val)? objectHeight: buttonHeight;
      heightforscrollers-=is_uid(val)? 0: buttonHeight;
      numberofscrollers+=is_uid(val)? 1: 0;
    }
    else
    if(object_property_is_list(user, pathkey)){
      uint8_t sz = object_property_size(user, pathkey);
      uint32_t wid=0;
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, pathkey, j);
        if(is_uid(val)){ wid=0; break; }
        wid += strlen(val)+1;
      }
      bool oneline=(wid >0 && wid < 30);
      int16_t h = oneline? buttonHeight: listHeight;
      heightforscrollers-=oneline? buttonHeight: 0;
      numberofscrollers+=oneline? 0: 1;
    }
  }
  return numberofscrollers? heightforscrollers/numberofscrollers: 0;
}

// ---------------

void GUI::drawObjectProperties(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
{
  drawObjectHeader(path, locallyEditable, width, depth);
  if(!open[depth]) return;
  int16_t scrollerheight=calculateScrollerHeight(path, height);
  int16_t keyWidth=calculateKeyWidth(path);
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s%s:", path, key);
    pathkey[l-1] = 0;
    if(!key) log_write("key=null: path=%s pathkey=%s i=%d size=%d is_value: %d value: %s\n", path, pathkey, i, size, object_property_is_value(user, pathkey), object_property_value(user, path, i));
    if(object_property_is_value(user, pathkey)){
      pathkey[l-1] = ':';
      char* val=object_property_value(user, path, i);
      int16_t height = is_uid(val)? scrollerheight: buttonHeight;
      if(height>=buttonHeight && key) drawPropertyValue(pathkey, key, val, locallyEditable, width, height, keyWidth, depth);
      else{
        char blnId[256]; snprintf(blnId, 256, "##filler %d %d %s", width, height, pathkey);
        ImGui::Button(blnId, ImVec2(width, paddingHeight));
        track_drag(blnId);
      }
    }
    else
    if(object_property_is_list(user, pathkey)){
      uint8_t sz = object_property_size(user, pathkey);
      uint32_t wid=0;
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, pathkey, j);
        if(is_uid(val)){ wid=0; break; }
        wid += strlen(val)+1;
      }
      int16_t height = (wid >0 && wid < 30)? buttonHeight: scrollerheight;
      if(height>=buttonHeight) drawPropertyList(pathkey, key, locallyEditable, width, height, keyWidth, depth);
      else{
        char blnId[256]; snprintf(blnId, 256, "##filler %d %d %s", width, height, pathkey);
        ImGui::Button(blnId, ImVec2(width, paddingHeight));
        track_drag(blnId);
      }
    }
  }
  drawObjectFooter(path, locallyEditable, width, keyWidth, depth);
}

void GUI::drawPropertyValue(char* path, char* key, char* val, bool locallyEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  if(width < 200) return;
  drawKey(path, key, width, height, keyWidth, depth);
  if(!is_uid(val)){
    drawNewPropertyValueEditor(path, val, true, locallyEditable, width-keyWidth, height, depth);
  }else{
    bool locallyEditable = object_is_local(val);
    drawNestedObjectProperties(path, locallyEditable, width-keyWidth, height, depth);
  }
}

void GUI::drawNestedObjectProperties(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
{
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, nodarken? listBackground: listBackgroundDark);
  ImGui::SameLine();
  ImGui::BeginChild(childName, ImVec2(width,height), true);
  {
    drawObjectProperties(path, locallyEditable, width-rhsPadding, height, depth+1);
    drawPadding(path, width-rhsPadding, paddingHeight, depth);
    if(locallyEditable) drawNewValueOrObjectButton(path, width-rhsPadding, 2, depth);
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::BeginChild(childName);
  set_drag_scroll(path);
  ImGui::End();
}

void GUI::drawPropertyList(char* path, char* key, bool locallyEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  if(width < 200) return;
  drawKey(path, key, width, height, keyWidth, depth);
  drawNestedObjectPropertiesList(path, locallyEditable, width-keyWidth, height, depth);
}

void GUI::drawKey(char* path, char* key, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? propertyBackground: propertyBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? propertyBackground: propertyBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
  char keyId[256]; snprintf(keyId, 256, "%s ## %s", key, path);
  ImGui::Button(keyId, ImVec2(keyWidth, height));
  track_drag(keyId);
  ImGui::PopStyleColor(4);
  ImGui::SameLine();
}

void GUI::drawNestedObjectPropertiesList(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
{
  bool oneline=(height==buttonHeight);
  bool multiln=false;
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  if(oneline){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  }
  bool nodarken=depth<3;
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, nodarken? listBackground: listBackgroundDark);
  ImGui::SameLine();
  ImGui::BeginChild(childName, ImVec2(width,height), true);
  {
    if(oneline){
      char onelinetext[512]; int n=0;
      uint8_t sz = object_property_size(user, path);
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, path, j);
        n+=snprintf(onelinetext+n, 512-n, "%s ", val);
      }
      size_t l=strlen(path);
      snprintf(path+l, 128-l, ":");
      drawNewPropertyValueEditor(path, onelinetext, true, locallyEditable, width, height, depth);
      path[l] = 0;
    }
    else{
      multiln=true;
      char multilinetext[512]; int n=0; int m=0;
      uint8_t sz = object_property_size(user, path);
      int j; for(j=1; j<=sz; j++){
        char* val=object_property_value(user, path, j);
        if(is_uid(val)){ multiln=false; break; }
        int l=snprintf(multilinetext+n, 512-n, "%s ", val);
        n+=l; m+=l;
        if(m>width/30){ n+=snprintf(multilinetext+n, 512-n, "\n"); m=0; }
      }
      if(multiln){
        size_t l=strlen(path);
        snprintf(path+l, 128-l, ":");
        drawNewPropertyValueEditor(path, multilinetext, true, locallyEditable, width, height, depth);
        drawPadding(path, width-rhsPadding, paddingHeight, depth);
        path[l] = 0;
      }
      else{
        uint8_t sz = object_property_size(user, path);
        size_t l=strlen(path);
        int j; for(j=1; j<=sz; j++){
          char* val=object_property_value(user, path, j);
          snprintf(path+l, 128-l, ":%d:", j);
          if(!is_uid(val)){
            drawNewPropertyValueEditor(path, val, false, locallyEditable, width-rhsPadding, buttonHeight, depth);
          }else{
            bool locallyEditable = object_is_local(val);
            drawObjectProperties(path, locallyEditable, width-rhsPadding, height, depth+1);
          }
          drawPadding(path, width-rhsPadding, paddingHeight, depth);
          path[l] = 0;
        }
        if(locallyEditable) drawNewValueOrObjectButton(path, width-rhsPadding, j, depth);
      }
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  if(!oneline && !multiln){
    ImGui::BeginChild(childName);
    set_drag_scroll(path);
    ImGui::End();
  }
  if(oneline){
    ImGui::PopStyleVar(1);
  }
}

// ---------------------------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------------------------

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
