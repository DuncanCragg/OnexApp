
extern "C" {
#include <items.h>
}

#include <imgui.h>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "im-gui.h"
#include "ux-features.h"
#include "display.h"

#pragma GCC diagnostic ignored "-Wformat-truncation"

extern void draw_padding(char* path, int16_t width, int16_t height, int8_t depth);

void draw_light(char* path, int16_t width, int16_t height)
{
  char pathlight[128]; snprintf(pathlight, 128, "%s:light", path);
  char pathname[128]; snprintf(pathname, 128, "%s:name", path);
  char* state=object_property(user, pathlight);
  char* name=object_property_values(user, pathname);
  char keyId[256]; snprintf(keyId, 256, "%s ## light %s", name? name: "light", path);
  if(!strcmp(state, "off")){
    ImGui::PushStyleColor(ImGuiCol_Button, valueBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackground);
    ImGui::Button(keyId, ImVec2(300, height));
    track_drag(keyId, true);
    ImGui::PopStyleColor(3);
  }
  else
  if(!strcmp(state, "on")){
    ImGui::PushStyleColor(ImGuiCol_Button, valueBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackgroundActive);
    ImGui::Button(keyId, ImVec2(300, height));
    track_drag(keyId, true);
    ImGui::PopStyleColor(3);
  }
}

bool buttonDown=false;

void draw_button(char* path, int16_t width, int16_t height)
{
  char pathstate[128]; snprintf(pathstate, 128, "%s:state", path);
  char pathname[128]; snprintf(pathname, 128, "%s:name", path);
  char* state=object_property(user, pathstate);
  char* name=object_property_values(user, pathname);
  char keyId[256]; snprintf(keyId, 256, "%s ## button %s", name? name: "button", path);
  if(!strcmp(state, "up")){
    ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackground);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
  }
  else
  if(!strcmp(state, "down")){
    ImGui::PushStyleColor(ImGuiCol_Button, propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackgroundActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
  }
  ImGui::Button(keyId, ImVec2(300, height));
  if(ImGui::IsItemActive()){
    if(!buttonDown){
      buttonDown=true;
      char* uid=object_property(user, path);
      invoke_single_set(uid, (char*)"state", (char*)"down");
    }
  }
  else{
    if(buttonDown){
      buttonDown=false;
      char* uid=object_property(user, path);
      invoke_single_set(uid, (char*)"state", (char*)"up");
    }
  }
  ImGui::PopStyleColor(3);
}

void draw_display(char* path, int16_t width, int16_t height)
{
  uint16_t ln = object_property_length(user, path);
  size_t l=strlen(path);
  int j; for(j=1; j<=ln; j++){
    char* uid=object_property_get_n(user, path, j);
    if(!is_uid(uid)) continue;
    snprintf(path+l, 128-l, ":%d", j);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, listBackground);
    char childName[128]; memcpy(childName, path, strlen(path)+1);
    ImGui::BeginChild(childName, ImVec2(width,height/3), true);
    {
      char pathis[128]; snprintf(pathis, 128, "%s:is", path);
      if(object_property_contains(user, pathis, (char*)"light")) draw_light(path, width, height/3);
      if(object_property_contains(user, pathis, (char*)"button")) draw_button(path, width, height/3);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(1);
    draw_padding(path, width, 100, 1);
    path[l] = 0;
  }
}

