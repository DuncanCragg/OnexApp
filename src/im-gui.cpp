
#include <imgui.h>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "im-gui.h"
#include "display.h"
#include "calendar.h"
#include "ux-features.h"

GUI* static_gui;

VulkanBase *app;

ImVec4 actionColour            (0.50f, 0.10f, 0.20f, 1.0f);
ImVec4 actionBackground        (0.96f, 0.96f, 0.87f, 1.0f);
ImVec4 actionBackgroundActive  (0.92f, 0.92f, 0.83f, 1.0f);

ImVec4 propertyColour          (0.20f, 0.50f, 0.30f, 1.0f);
ImVec4 propertyBackground      (0.97f, 1.00f, 0.98f, 1.0f);
ImVec4 propertyBackgroundActive(0.92f, 0.95f, 0.93f, 1.0f);

ImVec4 valueBackground         (0.96f, 0.87f, 1.00f, 1.0f);
ImVec4 valueBackgroundActive   (0.92f, 0.82f, 0.96f, 1.0f);

ImVec4 listBackground          (0.90f, 0.80f, 1.00f, 1.0f);
ImVec4 listBackgroundDark      (0.86f, 0.76f, 0.96f, 1.0f);

ImVec4 renderColour            (0.20f, 0.30f, 0.50f, 1.0f);
ImVec4 renderColourSoft        (0.65f, 0.70f, 0.80f, 1.0f);
ImVec4 renderBackground        (0.95f, 0.95f, 1.00f, 1.0f);
ImVec4 renderBackgroundActive  (0.90f, 0.90f, 0.95f, 1.0f);

ImVec4 ledOff(20.0f/255, 150.0f/255, 10.0f/255, 1.0f);
ImVec4 ledOn(100.0f/255, 255.0f/255, 50.0f/255, 1.0f);
ImVec4 btDisconnected(29.0f/255, 93.0f/255, 165.0f/255, 1.0f);
ImVec4 btConnected(   50.0f/255, 131.0f/255, 255.0f/255, 1.0f);

ImVec4 schemeBrown(183.0f/255, 142.0f/255, 96.0f/255, 1.0f);
ImVec4 schemeYellow(255.0f/255, 245.0f/255, 180.0f/255, 1.0f);
ImVec4 schemeMauve(221.0f/255, 190.0f/255, 243.0f/255, 1.0f);
ImVec4 schemePurple(169.0f/255, 103.0f/255, 212.0f/255, 1.0f);
ImVec4 schemeGreen(160.0f/255, 175.0f/255, 110.0f/255, 1.0f);
ImVec4 schemeLightPurple(0.8f, 0.7f, 0.9f, 1.0f);
ImVec4 schemeDarkerPurple(0.73f, 0.63f, 0.83f, 1.0f);
ImVec4 schemePlum(230.0f/255, 179.0f/255, 230.0f/255, 1.0f);

#define DARKEN_DEPTH 4

uint16_t buttonHeight=0;
uint16_t paddingHeight=0;
uint16_t objectHeight=0;
uint16_t listHeight=0;

uint16_t shorterValWidth=0;
uint16_t buttonWidth=0;
uint16_t smallButtonWidth=0;
uint16_t rhsPadding=0;

uint16_t workspace1Width;
uint16_t workspace1Height;
uint16_t workspace2Width;
uint16_t workspace2Height;

#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
uint16_t yOffsetTarget=0;
uint16_t yOffset=0;
uint16_t yOffsetCounter=0;
#endif

static bool rhsFullScreen=false;

bool displayView=false;
bool calendarView=false;
bool tableView=false;

char* propNameEditing=0;
bool  keyboardCancelled=false;

static ImGuiWindowFlags window_flags = 0;

unsigned char* fontData;
int texWidth, texHeight;

extern object* config;
extern object* user;
extern object* responses;
extern char* userUID;

bool evaluate_default(object* o, void* d)
{
  log_write("evaluate_default data=%p\n", d); object_log(o);
  return true;
}

void tick_user()
{
  if(userUID) onex_run_evaluators(userUID, 0);
}

extern bool front_end_running;

static void draw_window();

bool evaluate_user(object* o, void* d)
{
  if(front_end_running) draw_window();
  return true;
}

void init_imgui(float width, float height)
{
  ImGuiStyle& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
  style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
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
  style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // make it disappear..
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
//window_flags |= ImGuiWindowFlags_NoTitleBar;
//window_flags |= ImGuiWindowFlags_NoCollapse;
//window_flags |= ImGuiWindowFlags_MenuBar;
//window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
//window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//window_flags |= ImGuiWindowFlags_NoSavedSettings;
//window_flags |= ImGuiWindowFlags_NoInputs;
//window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
//window_flags |= ImGuiWindowFlags_AlwaysUseWindowPadding;
}

#if defined(__ANDROID__)
#define ASSET_PATH ""
#else
#define ASSET_PATH "./../data/"
#endif

#if defined(__ANDROID__)
static char* get_font_data(const char* fontfile, size_t* length)
{
  AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, fontfile, AASSET_MODE_STREAMING);
  *length = AAsset_getLength(asset);
  char* font_data = new char[*length]; // TODO free?
  AAsset_read(asset, font_data, *length);
  AAsset_close(asset);
  return font_data;
}
#endif

void get_font_info()
{
  ImGuiIO& io = ImGui::GetIO();
  const char* fontfilereg = ASSET_PATH "fonts/OpenSans-Regular.ttf";
  const char* fontfileemo = ASSET_PATH "fonts/OpenSansEmoji.ttf";
  static const ImWchar reg_range[] = { 0x0020, 0x00FF,  0 };
//static const ImWchar emo_range[] = { 0x1F300, 0x1F6FF,  0 }; // full range
  static const ImWchar emo_range[] = { 0x1F389,0x1F389, 0x1F4E5,0x1F4E7, 0x1F60D,0x1F60D, 0x1F686,0x1F686,  0 };
  ImFontConfig fontconf; fontconf.MergeMode = true;
  float fontsize = 60.0f;
#if defined(__ANDROID__)
  size_t lengthreg; char* fontdatareg=get_font_data(fontfilereg, &lengthreg);
  size_t lengthemo; char* fontdataemo=get_font_data(fontfileemo, &lengthemo);
  io.Fonts->AddFontFromMemoryTTF(fontdatareg, lengthreg, fontsize, 0,         reg_range);
  io.Fonts->AddFontFromMemoryTTF(fontdataemo, lengthemo, fontsize, &fontconf, emo_range);
#else
  io.Fonts->AddFontFromFileTTF(fontfilereg, fontsize, 0,         reg_range);
  io.Fonts->AddFontFromFileTTF(fontfileemo, fontsize, &fontconf, emo_range);
#endif
  io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
}

void set_scaling()
{
  float heightratio = ((float)app->height)/1350.0;
  float widthratio = ((float)app->width)/2000.0;

  buttonHeight=68*heightratio;
  paddingHeight=15*heightratio;
  objectHeight=400*heightratio;
  listHeight=1000*heightratio;

  shorterValWidth=680*widthratio;
  buttonWidth=190*widthratio;
  smallButtonWidth=65*widthratio;
  rhsPadding=20*widthratio;

  workspace1Width=((int)app->width)/2-10;
  workspace1Height=(int)app->height-70;
  workspace2Width=((int)app->width)/2-10;
  workspace2Height=(int)app->height;
}

// ---------------

object* get_or_create_edit(char* uid)
{
  object* edit;
  char* editUID=object_property(responses, uid);
  if(editUID){
    edit=onex_get_from_cache(editUID);
  }
  else{
    edit=object_new(0, 0, (char*)"edit rule", 5);

    object_property_set(edit, (char*)"edit-target", uid);
    object_property_set(edit, (char*)"edit-user",   userUID);
    object_property_add(edit, (char*)"Notifying",   uid);

    editUID=object_property(edit, (char*)"UID");
    object_property_set(responses, uid, editUID);
  }
  return edit;
}

void invoke_single_set(char* uid, char* key, char* val)
{
  char upd[128];
  if(strrchr(key, ':')){
    if(!val || !*val){
      snprintf(upd, 128, "=>");
    }
  }
  else snprintf(upd, 128, "=> %s", val? val: "");

  object* edit=get_or_create_edit(uid);

  char* oldkey=object_property_key(edit, (char*)":", 4);
  if(oldkey) object_property_set(edit, oldkey, (char*)"");
  object_property_set(edit, key, upd);

  if(false && is_local(uid) && !strcmp(key, (char*)"is")){
    char* evaluator;
    if(strstr(val, (char*)"light")) evaluator=(char*)"eval_light"; else
    if(strstr(val, (char*)"event")) evaluator=(char*)"eval_event";
    else                            evaluator=(char*)"eval_default";
    object* o = onex_get_from_cache(uid); // XXX
    object_set_evaluator(o, evaluator);
    log_write("set evaluator of is=%s to %s\n", val, evaluator);
    object_log(o);
  }
}

void invoke_single_add(char* uid, char* key, char* val) {

  if(!val || !*val) return;
  char upd[128]; snprintf(upd, 128, "=> @. %s", val);

  object* edit=get_or_create_edit(uid);

  char* oldkey=object_property_key(edit, (char*)":", 4);
  if(oldkey) object_property_set(edit, oldkey, (char*)"");
  object_property_set(edit, key, upd);
}

void set_new_tag(char* path, char* tag)
{
  char* lastcolon=strrchr(path,':'); *lastcolon=0;
  char* uid=object_property(user, path);
  object* objectEditing = onex_get_from_cache(uid);
  if(objectEditing){
    if(*tag){
      char tagpath[64]; snprintf(tagpath, 64, "taglookup:%s", tag);
      char* tagUID=object_property(config, tagpath);
      if(!tagUID) return;
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        invoke_single_set(uid, lastcolon+1, tagUID);
      }
      else invoke_single_add(uid, lastcolon+1, tagUID);
    }
    else{
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        invoke_single_set(uid, lastcolon+1, (char*)"");
      }
    }
  }
  *lastcolon=':';
}

void set_new_value(char* path, char* buf, bool single)
{
  if(single){
    char* lastcolon=strrchr(path,':'); *lastcolon=0;
    char* uid=object_property(user, path);
    invoke_single_set(uid, lastcolon+1, buf);
    *lastcolon=':';
  }
  else{
    char* lastcolon=strrchr(path,':'); *lastcolon=0;
    char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
    char* uid=object_property(user, path);
    *secondlastcolon=':'; *lastcolon=':';
    invoke_single_set(uid, secondlastcolon+1, buf);
  }
}

// ---------------

static object* create_new_object_for_property_name(char* path, char* name);

void set_property_name_and_object(char* path , char* name)
{
  char* uid=object_property(user, path);
  object* o = create_new_object_for_property_name(path, name);
  if(o) invoke_single_set(uid, name, object_property(o, (char*)"UID"));
  else invoke_single_set(uid, name, (char*)"--");
}

char* pop_last(char* path)
{
  uint16_t ln=object_property_length(user, path);
  char popPath[64]; snprintf(popPath, 64, "%s:%d", path, ln);
  char* last = object_property(user, popPath);
  object_property_set(user, popPath, 0);
  return last;
}

void set_property_name_and_link(char* path , char* name)
{
  char* lastlink=pop_last((char*)"viewing-r");
  if(!lastlink) return;
  char* uid=object_property(user, path);
  invoke_single_set(uid, name, lastlink);
}

// -------------

object* create_new_object_for_property_name(char* path, char* name)
{
  object* r=object_new(0, (char*)"eval_default", 0, 4);
  char* is;
  if(!strcmp(name,"Rules")) is=(char*)"Rule";
  else is=name;
  object_property_set(r, (char*)"is", is);
  return r;
}

object* create_new_object_like_others(char* path)
{
  object* r=0;
  int16_t ln = object_property_length(user, path);
  int8_t maxsz=0;
  for(int i=1; i<=ln; i++){
    size_t l=strlen(path);
    snprintf(path+l, 128-l, ":%d:", i);
    int8_t sz=object_property_size(user, path);
    if(sz>maxsz) maxsz=sz;
    path[l] = 0;
  }
  if(maxsz<4) maxsz=4;
  r=object_new(0, (char*)"eval_default", 0, maxsz);
  char* is=0;
  for(int i=ln; i>=1; i--){
    size_t l=strlen(path);
    snprintf(path+l, 128-l, ":%d:", i);
    int8_t sz=object_property_size(user, path);
    for(int j=1; j<=sz; j++){
      char* key=object_property_key(user, path, j);
      if(!strcmp(key,"is")){
        if(!is){
          is=object_property_val(user, path, j);
          object_property_set(r, (char*)"is", is);
        }
      }
      else{
        object_property_set(r, key, (char*)"--");
      }
    }
    path[l] = 0;
  }
  if(!is) object_property_set(r, (char*)"is", (char*)"editable");
  return r;
}

// ---------------

bool get_summary_from(char* path, char* summary, const char* key) {
  char pathkey[128]; snprintf(pathkey, 128, "%s:%s", path, key);
  char* vals=object_property_values(user, pathkey);
  if(!vals) return false;
  snprintf(summary, 128, "%s ", vals);
  return true;
}

void get_summary(char* path, char* summary) {
  *summary=0;
  if(get_summary_from(path, summary, "title")) return;
  if(get_summary_from(path, summary, "name")) return;
  if(get_summary_from(path, summary, "summary")) return;
  if(get_summary_from(path, summary, "description")) return;
  if(get_summary_from(path, summary, "text")) return;
  if(get_summary_from(path, summary, "content")) return;
  if(get_summary_from(path, summary, "is")) return;
  if(get_summary_from(path, summary, "Obstime")) return;
  if(get_summary_from(path, summary, "UID")) return;
}

// ---------------

int16_t calculate_key_width(char* path)
{
  int16_t w=0;
  char pathcolon[128]; snprintf(pathcolon, 128, "%s:", path);
  int8_t sz = object_property_size(user, pathcolon);
  if(sz>0) for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, pathcolon, i);
    if(key){
      int16_t l=strlen(key);
      if(l > w) w=l;
    }
  }
  return w>7? w*18: 196;
}

int16_t calculate_scroller_height(char* path, int16_t height)
{
  int16_t heightforscrollers=height-2.5*buttonHeight;
  int8_t  numberofscrollers=0;
  char pathcolon[128]; snprintf(pathcolon, 128, "%s:", path);
  int8_t sz = object_property_size(user, pathcolon);
  if(sz>0) for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, pathcolon, i);
    char pathkey[128]; snprintf(pathkey, 128, "%s:%s", path, key);
    uint16_t ln = object_property_length(user, pathkey);
    uint32_t wid=0;
    for(int j=1; j<=ln; j++){
      char* val=object_property_get_n(user, pathkey, j);
      if(is_uid(val)){ wid=0; break; }
      wid += strlen(val)+1;
    }
    int hgt;
    if(wid >0){ hgt=(wid/40+1)*buttonHeight; heightforscrollers-=hgt; }
    else        numberofscrollers++;
  }
  return (heightforscrollers >=0 && numberofscrollers) ? heightforscrollers/numberofscrollers: 0;
}

// ---------------

void draw_object_header(char* path, bool isEditable, int16_t width, int8_t depth)
{
  bool nodarken=depth<DARKEN_DEPTH;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, actionBackgroundActive);

  if(depth==1){
    char backId[256]; snprintf(backId, 256, " Back## %s", path);
    if(ImGui::Button(backId, ImVec2(buttonWidth, buttonHeight))){
      uint16_t histlen=object_property_length(user, (char*)"history");
      if(histlen){
        char popPath[64]; snprintf(popPath, 64, "history:%d", histlen);
        char* viewing = object_property(user, popPath);
        object_property_set(user, popPath, 0);
        object_property_set(user, (char*)"viewing-l", viewing);
        close_all_starting((char*)"viewing-l");
      }
    }
    track_drag(backId, true);
    ImGui::SameLine();
  }
  else if(depth<DARKEN_DEPTH){
    char dropId[256]; snprintf(dropId, 256, " X## %s", path);
    if(ImGui::Button(dropId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
      if(!strncmp(path, "viewing-l", strlen("viewing-l"))){
        char* lastcolon=strrchr(path,':'); *lastcolon=0;
        char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
        char* uid=object_property(user, path);
        *lastcolon=':'; *secondlastcolon=':';
        invoke_single_set(uid, secondlastcolon+1, (char*)"");
      }
      else{
        object_property_set(user, path, (char*)"");
        set_time_save_days();
      }
    }
    track_drag(dropId, true);
    ImGui::SameLine();
  }

  int blankwidth = width-(depth==1? buttonWidth: (depth<DARKEN_DEPTH? smallButtonWidth: 0))-2*smallButtonWidth;
  if(blankwidth>10){
    char summary[128]="";
    get_summary(path, summary);
    char barId[256]; snprintf(barId, 256, "%s ## topbar %s", summary, path);
    if(ImGui::Button(barId, ImVec2(blankwidth, buttonHeight)) && !dragPathId){
      toggle_open(path);
    }
    if(!linkTo) track_drag(barId, false);
    ImGui::SameLine();
    track_link(false, path, blankwidth, buttonHeight);
  }

  char maxId[256]; snprintf(maxId, 256, " ^## %s", path);
  if(ImGui::Button(maxId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
    char* viewing=object_property(user, path);
    object_property_add(user, (char*)"history", object_property(user, (char*)"viewing-l"));
    object_property_set(user, (char*)"viewing-l", viewing);
    close_all_starting((char*)"viewing-l");
  }
  track_drag(maxId, true);

  ImGui::SameLine();

  char pikId[256]; snprintf(pikId, 256, " >## %s", path);
  if(ImGui::Button(pikId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
    object_property_add(user, (char*)"viewing-r", object_property(user, path));
    set_time_save_days();
  }
  track_drag(pikId, true);

  ImGui::PopStyleColor(4);
  ImGui::PopStyleVar(1);
}

void draw_new_property_value_editor(char* path, char* propname, char* val, bool single, bool isEditable, int16_t width, int16_t height, int8_t depth)
{
  if(!val){ log_write("val==null: path=%s\n", path); return; }
  static int blurrer=0;
  char valId[256]; snprintf(valId, 256, "## val %s %s %d", val, path, blurrer);
  static char valBuf[256];
  ImStrncpy(valBuf, val, 256);
  ImGuiIO& io = ImGui::GetIO();
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(editing && (!io.WantTextInput || keyboardCancelled)){
    hide_keyboard();
    free(propNameEditing); propNameEditing=0;
    *valBuf=0;
    editing=false;
    blurrer++;
  }
  ImGui::PushItemWidth(width);
  if(depth){
    bool nodarken=depth<DARKEN_DEPTH;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, nodarken? valueBackground: valueBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, valueBackgroundActive);
  }
  int flags=ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine|ImGuiInputTextFlags_AutoSelectAll;
  if(!editing){
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, valueBackground);
    float multy=ImGui::GetCursorScreenPos().y-buttonHeight;
    if(height==buttonHeight){ ImGui::InputText(valId, valBuf, IM_ARRAYSIZE(valBuf), flags); multy=0; }
    else                      ImGui::InputTextMultiline(valId, valBuf, IM_ARRAYSIZE(valBuf), ImVec2(width, height), flags);
    if(ImGui::IsItemActive() && ImGui::IsMouseReleased(0) && !dragPathId){
      if(isEditable){
        propNameEditing = strdup(path);
        show_keyboard(multy);
      }
    }
    track_drag(valId, true);
    ImGui::PopStyleColor();
  }
  else{
    bool done=false;
    ImGuiTextEditCallback faa=filter_and_autocomplete_default;
    if(propname && !strcmp(propname, "tags")) faa=filter_and_autocomplete_calendar_tags;
    if(height==buttonHeight) done=filter_auto_input_text(valId, valBuf, IM_ARRAYSIZE(valBuf), faa);
    else                     done=ImGui::InputTextMultiline(valId, valBuf, IM_ARRAYSIZE(valBuf), ImVec2(width, height), flags);
    if(done){
      if(propname && !strcmp(propname, "tags")) set_new_tag(path, valBuf);
      else                                      set_new_value(path, valBuf, single);
      hide_keyboard();
      free(propNameEditing); propNameEditing=0;
      *valBuf=0;
    }
  }
  if(depth) ImGui::PopStyleColor(2);
  ImGui::PopItemWidth();
}

void draw_object_footer(char* path, bool isEditable, int16_t width, int16_t keyWidth, int8_t depth)
{
  if(depth>=3) return;
  if(!isEditable) return;
  static bool grabbedFocus=false;
  static char propNameBuf[256] = "";
  ImGuiIO& io = ImGui::GetIO();
  bool editing = propNameEditing && !strcmp(path, propNameEditing);
  if(editing && grabbedFocus && (!io.WantTextInput || keyboardCancelled)){
    hide_keyboard();
    free(propNameEditing); propNameEditing=0;
    *propNameBuf=0;
    grabbedFocus=false;
    editing=false;
  }
  if(!editing){
    char prpId[256]; snprintf(prpId, 256, " +## property %s", path);
    bool nodarken=depth<DARKEN_DEPTH;
    ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    ImGui::PushStyleColor(ImGuiCol_Button, nodarken? propertyBackground: propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? propertyBackground: propertyBackgroundActive);
    if(ImGui::Button(prpId, ImVec2(keyWidth, buttonHeight)) && !dragPathId){
      if(!propNameEditing){ propNameEditing = strdup(path); show_keyboard(0); }
    }
    ImGui::PopStyleColor(3);
    track_drag(prpId, true);

    ImGui::SameLine();

    int blankwidth = width - keyWidth;
    if(blankwidth>10){
      char barId[256]; snprintf(barId, 256, "## comboblank %s", path);
      ImGui::PushStyleColor(ImGuiCol_Button, nodarken? valueBackground: valueBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? valueBackground: valueBackgroundActive);
      ImGui::Button(barId, ImVec2(blankwidth, buttonHeight));
      ImGui::PopStyleColor(2);
      track_drag(barId, true);
    }
  }else{
    if(!grabbedFocus){
      ImGui::SetKeyboardFocusHere();
      grabbedFocus = io.WantTextInput;
    }
    ImGui::PushItemWidth(buttonWidth*2);
    ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, propertyBackground);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, propertyBackground);
    if(filter_auto_input_text("## property name", propNameBuf, IM_ARRAYSIZE(propNameBuf), filter_and_autocomplete_property_names)){
      if(*propNameBuf){
        if(!strcmp(propNameBuf, "Rules")) set_property_name_and_object(path, propNameBuf);
        else if(!strcmp(propNameBuf, "Notifying")) set_property_name_and_link(path, propNameBuf);
        else invoke_single_set(object_property(user, path), propNameBuf, (char*)"--");
        *propNameBuf=0;
      }
      hide_keyboard();
      free(propNameEditing); propNameEditing=0;
      grabbedFocus=false;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    int blankwidth = width-buttonWidth*2;
    if(blankwidth>10) ImGui::Button("## blank", ImVec2(blankwidth, buttonHeight));
  }
}

void draw_padding(char* path, int16_t width, int16_t height, int8_t depth)
{
  bool nodarken=depth<DARKEN_DEPTH;
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_Border, nodarken? listBackground: listBackgroundDark);
  ImGui::PushStyleColor(ImGuiCol_BorderShadow, nodarken? listBackground: listBackgroundDark);
  char blnId[256]; snprintf(blnId, 256, "##padding %d %d %s", width, height, path);
  ImGui::Button(blnId, ImVec2(width, height));
  track_drag(blnId, true);
  ImGui::PopStyleColor(4);
}

void draw_new_value_or_object_button(char* path, int16_t width, int j, int8_t depth, bool valueToo)
{
  bool nodarken=depth<DARKEN_DEPTH;
  char pathj[245]; snprintf(pathj, 245, "%s:%d", path, j);
  char* propname=strrchr(path, ':');
  if(propname) propname++;
  if(valueToo || (propname && !strcmp(propname, "tags"))){
    draw_new_property_value_editor(path, propname, (char*)"", true, true, (width-smallButtonWidth)/2, buttonHeight, depth);
    ImGui::SameLine();
  }

  ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? actionBackground: actionBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, actionBackgroundActive);

  int w=(width-smallButtonWidth)/(valueToo? 2: 1);
  char addObjId[256]; snprintf(addObjId, 256, "+object ## %s", pathj);
  if(ImGui::Button(addObjId, ImVec2(w, buttonHeight)) && !dragPathId){
    char* lastcolon=strrchr(path,':'); *lastcolon=0;
    char* uid=object_property(user, path);
    object* objectEditing = onex_get_from_cache(uid);
    *lastcolon=':';
    object* o = create_new_object_like_others(path);
    if(o){
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        invoke_single_set(uid, lastcolon+1, object_property(o, (char*)"UID"));
      }
      else invoke_single_add(uid, lastcolon+1, object_property(o, (char*)"UID"));
    }
  }
  if(!linkFrom) track_drag(addObjId, false);
  ImGui::SameLine();
  track_link(true, path, 0, 0);

  char addLnkId[256]; snprintf(addLnkId, 256, " <## %s", pathj);
  if(ImGui::Button(addLnkId, ImVec2(smallButtonWidth, buttonHeight)) && !dragPathId){
    char* lastlink=pop_last((char*)"viewing-r");
    if(lastlink){
      char* lastcolon=strrchr(path,':'); *lastcolon=0;
      char* uid=object_property(user, path);
      object* objectEditing = onex_get_from_cache(uid);
      *lastcolon=':';
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        invoke_single_set(uid, lastcolon+1, lastlink);
      }
      else invoke_single_add(uid, lastcolon+1, lastlink);
    }
  }
  if(!linkFrom) track_drag(addLnkId, false);
  ImGui::SameLine();
  track_link(true, path, w+smallButtonWidth, buttonHeight);

  ImGui::PopStyleColor(4);
}

void draw_key(char* path, char* key, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  bool nodarken=depth<DARKEN_DEPTH;
  ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
  ImGui::PushStyleColor(ImGuiCol_Button, nodarken? propertyBackground: propertyBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodarken? propertyBackground: propertyBackgroundActive);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
  char keyId[256]; snprintf(keyId, 256, "%s ## key %s", key, path);
  ImGui::Button(keyId, ImVec2(keyWidth, height));
  track_drag(keyId, true);
  ImGui::PopStyleColor(4);
  ImGui::SameLine();
}

static void draw_nested_object_properties_list(char* path, bool isEditable, int16_t width, int16_t height, int8_t depth);

void draw_property_list(char* path, char* key, bool isEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  if(width < 200) return;
  draw_key(path, key, width, height, keyWidth, depth);
  draw_nested_object_properties_list(path, isEditable, width-keyWidth, height, depth);
}

void draw_property(char* key, char* path, bool isEditable, int16_t width, int16_t height, int8_t depth, int16_t scrollerheight, int16_t keyWidth)
{
  if(!key){ log_write("key==null: path=%s\n", path); return; }
  char pathkey[128]; snprintf(pathkey, 128, "%s:%s", path, key);
  uint16_t ln = object_property_length(user, pathkey);
  if(!ln) return;
  uint32_t wid=0;
  for(int j=1; j<=ln; j++){
    char* val=object_property_get_n(user, pathkey, j);
    if(is_uid(val)){ wid=0; break; }
    wid += strlen(val)+1;
  }
  int hgt;
  if(wid >0) hgt=(wid/40+1)*buttonHeight;
  else       hgt=scrollerheight;
  if(hgt>=buttonHeight) draw_property_list(pathkey, key, isEditable, width, hgt, keyWidth, depth);
  else{
    char blnId[256]; snprintf(blnId, 256, "##filler %d %d %s", width, hgt, pathkey);
    ImGui::Button(blnId, ImVec2(width, paddingHeight));
    track_drag(blnId, true);
  }
}

void draw_object_properties(char* path, bool isEditable, int16_t width, int16_t height, int8_t depth)
{
  draw_object_header(path, isEditable, width, depth);

  if(strcmp(path, "viewing-l") && !is_open(path)) return;

  int16_t scrollerheight=calculate_scroller_height(path, height);
  int16_t keyWidth=calculate_key_width(path);

  draw_property((char*)"Timer", path, isEditable, width, height, depth, scrollerheight, keyWidth);

  char pathcolon[128]; snprintf(pathcolon, 128, "%s:", path);
  int8_t sz = object_property_size(user, pathcolon);
  for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, pathcolon, i);
    draw_property(key, path, isEditable, width, height, depth, scrollerheight, keyWidth);
  }
  draw_object_footer(path, isEditable, width, keyWidth, depth);
}

void draw_nested_object_properties_list(char* path, bool isEditable, int16_t width, int16_t height, int8_t depth)
{
  bool oneline=(height==buttonHeight);
  bool multiln=false;
  bool newline=false;
  char textlines[512]=""; int n=0;
  if(oneline){
    if(object_property_is(user, path, (char*)"--")){
      oneline=false; newline=true;
    }
    else{
      uint16_t ln = object_property_length(user, path);
      for(int j=1; j<=ln; j++){
        char* val=object_property_get_n(user, path, j);
        n+=snprintf(textlines+n, 512-n, "%s ", val);
      }
    }
  }
  else{
    multiln=true;
    uint16_t ln = object_property_length(user, path);
    int m=0;
    int j; for(j=1; j<=ln; j++){
      char* val=object_property_get_n(user, path, j);
      if(is_uid(val)){ multiln=false; break; }
      int l=snprintf(textlines+n, 512-n, "%s ", val);
      n+=l; m+=l;
      if(m*30>width){ textlines[n-1]='\n'; m=0; }
    }
  }
  if(oneline || multiln || newline){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  }
  bool nodarken=depth<DARKEN_DEPTH;
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, nodarken? listBackground: listBackgroundDark);
  ImGui::SameLine();
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  ImGui::BeginChild(childName, ImVec2(width,height), true);
  {
    if(oneline || multiln){
      if(n) textlines[n-1]=0;
      draw_new_property_value_editor(path, 0, textlines, true, isEditable, width, height, depth);
    }
    else
    if(newline){
      draw_new_value_or_object_button(path, width, 1, depth, true);
    }
    else{
      uint16_t ln = object_property_length(user, path);
      size_t l=strlen(path);
      int j; for(j=1; j<=ln; j++){
        char* val=object_property_get_n(user, path, j);
        snprintf(path+l, 128-l, ":%d", j);
        if(!is_uid(val)){
          draw_new_property_value_editor(path, 0, val, false, isEditable, width-rhsPadding, buttonHeight, depth);
        }else{
          char pathis[128]; snprintf(pathis, 128, "%s:is", path);
          bool isEditable = is_local(val) || object_property_contains(user, pathis, (char*)"editable");
          draw_object_properties(path, isEditable, width-rhsPadding, height, depth+1);
        }
        draw_padding(path, width-rhsPadding, paddingHeight, depth);
        path[l] = 0;
      }
      if(isEditable) draw_new_value_or_object_button(path, width-rhsPadding, j, depth, true);
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  if(oneline || multiln || newline){
    ImGui::PopStyleVar(1);
  }
  else{
    ImGui::BeginChild(childName);
    set_drag_scroll(path);
    ImGui::EndChild();
  }
}

void draw_view()
{
  int msperframe = (int)(1000.0f/ImGui::GetIO().Framerate);
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  if(yOffsetCounter > 0){
    yOffset=yOffsetTarget*(100-yOffsetCounter)/100;
    yOffsetCounter-=msperframe >10? 10 : 5;
  }
#endif
  if(!rhsFullScreen){
    ImGui::BeginChild("Workspace1", ImVec2(workspace1Width,workspace1Height), true);
    {
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
      ImVec2 startingpoint = ImGui::GetCursorScreenPos();
      ImVec2 startpos(startingpoint.x, startingpoint.y - yOffset);
      ImGui::SetCursorScreenPos(startpos);
#endif
      int8_t s=strlen("viewing-l")+1;
      char path[s]; memcpy(path, "viewing-l", s);
      char* uid=object_property(user, (char*)"viewing-l");
      bool isEditable = is_local(uid) || object_property_contains(user, (char*)"viewing-l:is", (char*)"editable");
      draw_object_properties(path, isEditable, workspace1Width-rhsPadding, workspace1Height, 1);
    }
    ImGui::EndChild();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(-1,0));
    ImGui::SameLine();
    ImGui::PopStyleVar();
  }
  uint16_t ws2width=rhsFullScreen? workspace1Width+workspace2Width: workspace2Width;
  ImGui::BeginChild("Workspace2", ImVec2(ws2width,workspace2Height), true);
  {
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
    ImVec2 startingpoint = ImGui::GetCursorScreenPos();
    ImVec2 startpos(startingpoint.x, startingpoint.y - yOffset);
    ImGui::SetCursorScreenPos(startpos);
#endif
    ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    ImGui::PushStyleColor(ImGuiCol_Button, actionBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, actionBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, actionBackground);

    if(displayView) ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    else            ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    if(ImGui::Button(" display", ImVec2(buttonWidth/2+smallButtonWidth, buttonHeight)))
    {
      displayView=!displayView;
      if(displayView){
        tableView=false;
        calendarView=false;
        close_all_starting((char*)"viewing-r");
      }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if(calendarView) ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    else             ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    if(ImGui::Button(" calendar", ImVec2(buttonWidth/2+smallButtonWidth, buttonHeight)))
    {
      calendarView=!calendarView;
      if(calendarView){
        displayView=false;
        tableView=false;
        close_all_starting((char*)"viewing-r");
        set_time_save_days();
      }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if(tableView) ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    else          ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    if(ImGui::Button(" table", ImVec2(buttonWidth/2+smallButtonWidth/2, buttonHeight)))
    {
      tableView=!tableView;
      if(tableView){
        displayView=false;
        calendarView=false;
        close_all_starting((char*)"viewing-r");
      }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::Button("##paddingbutton", ImVec2(ws2width-2*buttonWidth-2.5*smallButtonWidth-rhsPadding, buttonHeight));

    ImGui::SameLine();

    if(ImGui::Button(rhsFullScreen? " >>" : " <<", ImVec2(smallButtonWidth*1.5, buttonHeight)))
    {
      rhsFullScreen=!rhsFullScreen;
    }
    ImGui::PopStyleColor(4);

    ImGui::Separator(); // needed to reset to LHS?

    int8_t s=strlen("viewing-r")+1; char path[s]; memcpy(path, "viewing-r", s);
    if(displayView) draw_display(path, ws2width-rhsPadding, workspace2Height);
    else
    if(calendarView) draw_calendar(path, ws2width-rhsPadding, workspace2Height-3*buttonHeight);
    else
    if(tableView);// draw_table(..);
    else             draw_nested_object_properties_list(path, false, ws2width-rhsPadding, workspace2Height-3*buttonHeight, 1);
  }
  ImGui::EndChild();
  draw_link();
}

bool render_content_ready=false;

void draw_window()
{
  if(render_content_ready) return;
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2((float)app->width, (float)app->height), ImGuiSetCond_FirstUseEver);

  if(!ImGui::Begin("Onex", NULL, window_flags)){
      ImGui::End();
      ImGui::Render();
      render_content_ready=true;
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
//style.TouchExtraPadding = ImVec2(10.0f,10.0f);

  draw_view();

  ImGui::PopStyleVar(svs);

  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(650, 650), ImGuiSetCond_FirstUseEver);
//ImGui::ShowTestWindow();

  ImGui::Render();
  render_content_ready=true;
}
