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

ImVec4 actionTextColour(0.5f, 0.1f, 0.2f, 1.0f);
ImVec4 actionTextBG(0.90f, 0.90f, 0.85f, 1.0f);
ImVec4 propertyColour(0.2f, 0.5f, 0.3f, 1.0f);
ImVec4 keywordColour(0.5f, 0.1f, 0.2f, 1.0f);
ImVec4 propertyBackground(0.90f, 0.90f, 1.00f, 1.00f);
ImVec4 valueBackground(0.9f, 0.8f, 1.0f, 1.0f);

ImVec4 schemeBrown(183.0f/255, 142.0f/255, 96.0f/255, 1.0f);
ImVec4 schemeYellow(255.0f/255, 245.0f/255, 180.0f/255, 1.0f);
ImVec4 schemeMauve(221.0f/255, 190.0f/255, 243.0f/255, 1.0f);
ImVec4 schemePurple(169.0f/255, 103.0f/255, 212.0f/255, 1.0f);
ImVec4 schemeGreen(160.0f/255, 175.0f/255, 110.0f/255, 1.0f);
ImVec4 schemeLightPurple(0.8f, 0.7f, 0.9f, 1.0f);
ImVec4 schemePlum(230.0f/255, 179.0f/255, 230.0f/255, 1.0f);

#define keyWidth 280
#define shorterValWidth 680
#define objectHeight 400
#define listHeight 1000
#define buttonWidth 190
#define smallButtonWidth 90
#define buttonHeight 70
#define listBackground schemeLightPurple
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
//style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_ComboBg] = actionTextBG;
  style.Colors[ImGuiCol_FrameBg] = actionTextBG;
//style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.9f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_Button] = valueBackground;
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.88f, 0.78f, 0.98f, 1.0f);
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

  int svs = 5;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10,5));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f,0));

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

const char* propNameStrings[] = { "+", "+value", "+object", "title", "description", "Rules", "Notifying", "Alerted", "Timer" };
int         propNameChoice = 0;
char*       propNameEditing=0;
uint16_t    yOffsetTarget=0;
uint16_t    yOffset=0;
uint16_t    yOffsetCounter=0;

void GUI::showKeyboard(){
#if defined(__ANDROID__)
  float y=ImGui::GetCursorScreenPos().y;
  yOffsetTarget=y/1.85-40;
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
    if(user) drawObjectProperties(path, locallyEditable, workspace1Width-rhsPadding, workspace1Height);
  }
  ImGui::EndChild();
}

static bool evaluate_any_object(object* user)
{
  return true;
}

static char*  drag_path=0;
static float delta_x=0.0f;
static float delta_y=0.0f;
static bool drag_handled = false;
#define MOVING_DELTA(x,y) (((x)*(x)+(y)*(y)) > 0.001f)

static void track_drag(char* path)
{
  if(ImGui::IsItemActive() && ImGui::IsMouseDragging()){
    if(!drag_path || strcmp(drag_path, path)) drag_path=strdup(path);
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    if(MOVING_DELTA(mouse_delta.x, mouse_delta.y)){
      delta_x=mouse_delta.x;
      delta_y=mouse_delta.y;
      drag_handled=false;
    }
  }
  else
  if(MOVING_DELTA(delta_x, delta_y) && drag_path && !strcmp(path, drag_path)){
    delta_x *= 0.99f;
    delta_y *= 0.99f;
    drag_handled=false;
  }
  else
  if(!ImGui::IsMouseDragging() && drag_path && !strcmp(path, drag_path)){
    free(drag_path);
    drag_path=0;
  }
}

static void set_drag_scroll(char* path)
{
  if(drag_path && !strncmp(drag_path, path, strlen(path)) && strcmp(drag_path, path) && !drag_handled && MOVING_DELTA(delta_x,delta_y)){
    ImGui::SetScrollX(ImGui::GetScrollX() - delta_x);
    ImGui::SetScrollY(ImGui::GetScrollY() - delta_y);
    drag_handled=true;
  }
}

void GUI::setPropertyName(char* path , char* name)
{
  char* lastcolon=strrchr(path,':');
  *lastcolon=0;
  object* objectEditing = object_get_from_cache(object_property(user, path));
  *lastcolon=':';
  object_property_set(objectEditing, name, (char*)"--");
  free(propNameEditing); propNameEditing=0;
  propNameChoice = 0;
}

void GUI::setPropertyNameAndObject(char* path , char* name)
{
  char* lastcolon=strrchr(path,':');
  *lastcolon=0;
  object* objectEditing = object_get_from_cache(object_property(user, path));
  *lastcolon=':';
  object* o = createNewObjectForPropertyName(path, name);
  if(o) object_property_set(objectEditing, name, object_property(o, (char*)"UID"));
  else object_property_set(objectEditing, name, (char*)"---");
  free(propNameEditing); propNameEditing=0;
  propNameChoice = 0;
}

void GUI::drawNewPropertyValueEditor(char* path, char* val, bool single, bool locallyEditable, int16_t width, int16_t height)
{
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(!editing){
    char valId[256]; snprintf(valId, 256, "%s ## %s", val, path);
    if(ImGui::Button(valId, ImVec2(width, height))){
      if(locallyEditable && (!drag_path || strcmp(path, drag_path))){
        propNameEditing = strdup(path);
        showKeyboard();
      }
    }
    track_drag(path);
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
    ImGui::PushItemWidth(width);
    if(ImGui::InputText("## property value", b, 64, ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue, TextFilters::FilterImGuiLetters)){
      char* lastcolon=strrchr(path,':'); *lastcolon=0;
      if(single){
        char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
        object* objectEditing = object_get_from_cache(object_property(user, path));
        if(!*b) object_property_set(objectEditing, secondlastcolon+1, (char*)"");
        else object_property_set(objectEditing, secondlastcolon+1, strdup(b));
        *secondlastcolon=':';
      }
      else{
        char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
        char* thirdlastcolon=strrchr(path, ':'); *thirdlastcolon=0;
        object* objectEditing = object_get_from_cache(object_property(user, path));
        *secondlastcolon=':';
        if(!*b) object_property_set(objectEditing, thirdlastcolon+1, (char*)"");
        else object_property_set(objectEditing, thirdlastcolon+1, strdup(b));
        *thirdlastcolon=':';
      }
      *lastcolon=':';
      free(propNameEditing); propNameEditing=0;
      hideKeyboard();
      *b=0;
    }
    ImGui::PopItemWidth();
  }
}

static void drawPadding(char* path, int16_t width, int16_t height)
{
  ImGui::PushStyleColor(ImGuiCol_Button, listBackground);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, listBackground);
  ImGui::PushStyleColor(ImGuiCol_Border, listBackground);
  ImGui::PushStyleColor(ImGuiCol_BorderShadow, listBackground);
  ImGui::Button("## blank", ImVec2(width, height));
  ImGui::PopStyleColor(4);
  track_drag(path);
}

void GUI::drawNewValueOrObjectButtons(char* path, int16_t width)
{
  ImGui::PushStyleColor(ImGuiCol_Text, actionTextColour);
  ImGui::PushStyleColor(ImGuiCol_Button, actionTextBG);
  drawPadding(path, width-smallButtonWidth, paddingHeight);
  ImGui::SameLine();
  char addObjId[256]; snprintf(addObjId, 256, "+##%s", path);
  if(ImGui::Button(addObjId, ImVec2(smallButtonWidth, buttonHeight))){
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
    object* v = object_get_from_cache(object_property(user, path));
    if(secondlastcolon) *secondlastcolon=':';
    *lastcolon=':';
    object* o = createNewObjectLikeOthers(path);
    if(o) object_property_add(v, vkey, object_property(o, (char*)"UID"));
    free(vkey);
  }
  track_drag(path);
  ImGui::PopStyleColor(2);
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

void GUI::drawObjectHeader(char* path, bool locallyEditable, int16_t width)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  ImGui::PushStyleColor(ImGuiCol_Text, actionTextColour);
  ImGui::PushStyleColor(ImGuiCol_Button, actionTextBG);

  bool topobj=!strcmp(path, "viewing:");
  if(topobj){
    char linkId[256]; snprintf(linkId, 256, "<<##%s", path);
    if(ImGui::Button(linkId, ImVec2(smallButtonWidth, buttonHeight))){
      uint16_t histlen=object_property_size(user, (char*)"history");
      char* viewing = object_property_value(user, (char*)"history", histlen);
      char popPath[64]; snprintf(popPath, 64, "history:%d:", histlen);
      object_property_set(user, (char*)popPath, 0);
      object_property_set(user, (char*)"viewing", viewing);
    }
    track_drag(path);
    ImGui::SameLine();
  }

  int blankwidth = width-(topobj? 3: 2)*smallButtonWidth;
  if(blankwidth>10){
    ImGui::Button("## blank", ImVec2(blankwidth, buttonHeight));
    track_drag(path);
    ImGui::SameLine();
  }

  char maxId[256]; snprintf(maxId, 256, "[+]##%s", path);
  if(ImGui::Button(maxId, ImVec2(smallButtonWidth, buttonHeight))){
    char* lastcolon=strrchr(path,':');
    *lastcolon=0;
    char* viewing=object_property(user, path);
    *lastcolon=':';
    object_property_add(user, (char*)"history", object_property(user, (char*)"viewing"));
    object_property_set(user, (char*)"viewing", viewing);
  }
  track_drag(path);

  ImGui::SameLine();

  char dropId[256]; snprintf(dropId, 256, "X##%s", path);
  if(ImGui::Button(dropId, ImVec2(smallButtonWidth, buttonHeight))){
    char* lastcolon=strrchr(path,':'); *lastcolon=0;
    char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
    char* thirdlastcolon=strrchr(path, ':'); *thirdlastcolon=0;
    object* objectEditing = object_get_from_cache(object_property(user, path));
    *secondlastcolon=':';
    object_property_set(objectEditing, thirdlastcolon+1, (char*)"");
    *thirdlastcolon=':';
    *lastcolon=':';
  }
  track_drag(path);

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(1);
}

void GUI::drawNewPropertyCombo(char* path, int16_t width)
{
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(!editing){
    ImGui::PushItemWidth(keyWidth);
    int c=0;
    char id[128]; snprintf(id, 128, "## %s", path);
    ImGui::Combo(id, !propNameEditing? &propNameChoice: &c, propNameStrings, IM_ARRAYSIZE(propNameStrings));
    track_drag(path);
    if(!propNameEditing && propNameChoice){ propNameEditing = strdup(path); if(propNameChoice==1 || propNameChoice==2) showKeyboard(); }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    int blankwidth = width - keyWidth;
    if(blankwidth>10){
      ImGui::Button("## blank", ImVec2(blankwidth, buttonHeight));
      track_drag(path);
    }
  }else{
    if(propNameChoice > 2){
      char* propname=(char*)propNameStrings[propNameChoice];
      if(!strcmp(propname, "Rules")) setPropertyNameAndObject(path, propname);
      else setPropertyName(path, propname);
    }
    else if(propNameChoice==1){
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
      ImGui::PushItemWidth(keyWidth);
      if(ImGui::InputText("## property name", b, 64, ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue, TextFilters::FilterImGuiLetters)){
        setPropertyName(path, strdup(b));
        hideKeyboard();
        *b=0;
      }
      ImGui::PopItemWidth();
      ImGui::SameLine();
      int blankwidth = width - keyWidth;
      if(blankwidth>10) ImGui::Button("--## blank", ImVec2(blankwidth, buttonHeight));
    }
    else if(propNameChoice==2){
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
      ImGui::PushItemWidth(keyWidth);
      if(ImGui::InputText("## property name", b, 64, ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_EnterReturnsTrue, TextFilters::FilterImGuiLetters)){
        setPropertyNameAndObject(path, strdup(b));
        hideKeyboard();
        *b=0;
      }
      ImGui::PopItemWidth();
      ImGui::SameLine();
      int blankwidth = width - keyWidth;
      if(blankwidth>10) ImGui::Button("[]## blank", ImVec2(blankwidth, buttonHeight));
    }
  }
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
      int16_t h = (wid >0 && wid < 20)? buttonHeight: listHeight;
      heightforscrollers-=(wid >0 && wid < 20)? buttonHeight: 0;
      numberofscrollers+=(wid >0 && wid < 20)? 0: 1;
    }
  }
  return numberofscrollers? heightforscrollers/numberofscrollers: 0;
}

// ---------------

void GUI::drawObjectProperties(char* path, bool locallyEditable, int16_t width, int16_t height)
{
  drawObjectHeader(path, locallyEditable, width);
  if(width < 200) return;
  int16_t scrollerheight=calculateScrollerHeight(path, height);
  uint8_t size = object_property_size(user, path);
  for(int i=1; i<=size; i++){
    char* key=object_property_key(user, path, i);
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s%s:", path, key);
    pathkey[l-1] = 0;
    if(object_property_is_value(user, pathkey)){
      pathkey[l-1] = ':';
      char* val=object_property_value(user, path, i);
      int16_t height = is_uid(val)? scrollerheight: buttonHeight;
      if(height>=buttonHeight) drawPropertyValue(pathkey, key, val, locallyEditable, width-keyWidth, height);
      else{
        ImGui::Button("## blank", ImVec2(width, paddingHeight));
        track_drag(path);
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
      int16_t height = (wid >0 && wid < 20)? buttonHeight: scrollerheight;
      if(height>=buttonHeight) drawPropertyList(pathkey, key, locallyEditable, width-keyWidth, height);
      else{
        ImGui::Button("## blank", ImVec2(width, paddingHeight));
        track_drag(path);
      }
    }
  }
  if(locallyEditable) drawNewPropertyCombo(path, width);
}

void GUI::drawPropertyValue(char* path, char* key, char* val, bool locallyEditable, int16_t width, int16_t height)
{
  if(width < 200) return;
  ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
  ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
  char keyId[256]; snprintf(keyId, 256, "%s ## %s", key, path);
  ImGui::Button(keyId, ImVec2(keyWidth, height));
  track_drag(path);
  ImGui::PopStyleColor(2);
  ImGui::SameLine();
  if(!is_uid(val)){
    drawNewPropertyValueEditor(path, val, true, locallyEditable, width, height);
  }else{
    bool locallyEditable = object_is_local(val);
    drawNestedObjectProperties(path, locallyEditable, width, height);
  }
}

void GUI::drawNestedObjectProperties(char* path, bool locallyEditable, int16_t width, int16_t height)
{
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, listBackground);
  ImGui::SameLine();
  ImVec2 start_draggable_pos = ImGui::GetCursorScreenPos();
  ImGui::BeginChild(childName, ImVec2(width,height), true);
  {
    drawObjectProperties(path, locallyEditable, width-rhsPadding, height);
    drawPadding(path, width-rhsPadding, paddingHeight);
    if(locallyEditable) drawNewValueOrObjectButtons(path, width-rhsPadding);

    ImVec2 end_draggable_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(end_draggable_pos.x-start_draggable_pos.x, end_draggable_pos.y-start_draggable_pos.y);
    ImGui::SetCursorScreenPos(start_draggable_pos);
    ImGui::PushID(childName);
    ImGui::InvisibleButton("dragme", canvas_size);
    ImGui::PopID();
    track_drag(path);
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::BeginChild(childName);
  set_drag_scroll(path);
  ImGui::End();
}

void GUI::drawPropertyList(char* path, char* key, bool locallyEditable, int16_t width, int16_t height)
{
  if(width < 200) return;
  ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
  ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
  char keyId[256]; snprintf(keyId, 256, "%s ## %s", key, path);
  ImGui::Button(keyId, ImVec2(keyWidth, height));
  track_drag(path);
  ImGui::PopStyleColor(2);
  ImGui::SameLine();
  drawNestedObjectPropertiesList(path, locallyEditable, width, height);
}

void GUI::drawNestedObjectPropertiesList(char* path, bool locallyEditable, int16_t width, int16_t height)
{
  bool oneline=(height==buttonHeight);
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  if(oneline){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  }
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, listBackground);
  ImGui::SameLine();
  ImVec2 start_draggable_pos = ImGui::GetCursorScreenPos();
  ImGui::BeginChild(childName, ImVec2(width,height), true);
  {
    if(oneline){
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20,5));
      char text[512]; int n=0;
      uint8_t sz = object_property_size(user, path);
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, path, j);
        n+=snprintf(text+n, 512-n, "%s ", val);
      }
      size_t l=strlen(path);
      snprintf(path+l, 128-l, ":");
      drawNewPropertyValueEditor(path, text, true, locallyEditable, width, buttonHeight);
      path[l] = 0;
    }
    else{
      uint8_t sz = object_property_size(user, path);
      for(int j=1; j<=sz; j++){
        char* val=object_property_value(user, path, j);
        size_t l=strlen(path);
        snprintf(path+l, 128-l, ":%d:", j);
        if(!is_uid(val)){
          drawNewPropertyValueEditor(path, val, false, locallyEditable, width-rhsPadding, buttonHeight);
        }else{
          bool locallyEditable = object_is_local(val);
          drawObjectProperties(path, locallyEditable, width-rhsPadding, height);
        }
        path[l] = 0;
        drawPadding(path, width-rhsPadding, paddingHeight);
      }
      if(locallyEditable){
        drawNewValueOrObjectButtons(path, width-rhsPadding);
      }
    }

    ImVec2 end_draggable_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(end_draggable_pos.x-start_draggable_pos.x, end_draggable_pos.y-start_draggable_pos.y);
    ImGui::SetCursorScreenPos(start_draggable_pos);
    ImGui::PushID(childName);
    ImGui::InvisibleButton("dragme", canvas_size);
    ImGui::PopID();
    track_drag(path);
    if(oneline){
      ImGui::PopStyleVar(1);
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::BeginChild(childName);
  set_drag_scroll(path);
  ImGui::End();
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
