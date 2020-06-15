
extern "C" {
#include <items.h>
}

#include <imgui.h>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "im-gui.h"
#include "ux-features.h"
#include "display.h"

#define OUTER_PADDING 20

extern void draw_padding(char* path, int16_t width, int16_t height, int8_t depth);
extern void set_new_value(char* path, char* buf, bool single);

void draw_light(char* path, int16_t width)
{
  char pathlight[128]; snprintf(pathlight, 128, "%s:light", path);
  char pathname[128]; snprintf(pathname, 128, "%s:name", path);
  char* state=object_property(user, pathlight);
  char* name=object_property_values(user, pathname);
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::BeginChild(childName, ImVec2(width,200), true);
  {
    if(state && !strcmp(state, "off")){
      ImGui::PushStyleColor(ImGuiCol_Border, ledOff);
      ImGui::PushStyleColor(ImGuiCol_Button, ledOff);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ledOff);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ledOff);
    }
    else
    if(state && !strcmp(state, "on")){
      ImGui::PushStyleColor(ImGuiCol_Border, ledOff);
      ImGui::PushStyleColor(ImGuiCol_Button, ledOn);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ledOn);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ledOn);
    }
    else {
      ImGui::PushStyleColor(ImGuiCol_Border, valueBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_Button, valueBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackground);
    }
    char keyId[256]; snprintf(keyId, 256, "%s ## light %s", name? name: "light", path);
    if(ImGui::Button(keyId, ImVec2(200, 200-OUTER_PADDING*2))){
      set_new_value(pathlight, (char*)"on", true);
    }
    ImGui::PopStyleColor(4);
  }
  ImGui::SameLine();
  ImGui::EndChild();
  ImGui::PopStyleColor(2);
}

char* buttonDown;

void draw_button(char* path, int16_t width)
{
  char pathstate[128]; snprintf(pathstate, 128, "%s:state", path);
  char pathname[128]; snprintf(pathname, 128, "%s:name", path);
  char* state=object_property(user, pathstate);
  char* name=object_property_values(user, pathname);
  char childName[128]; memcpy(childName, path, strlen(path)+1);
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::BeginChild(childName, ImVec2(width,100), true);
  {
    if(state && !strcmp(state, "up")){
      ImGui::PushStyleColor(ImGuiCol_Border, renderColourSoft);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
    }
    else
    if(state && !strcmp(state, "down")){
      ImGui::PushStyleColor(ImGuiCol_Border, renderColourSoft);
      ImGui::PushStyleColor(ImGuiCol_Button, propertyBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
    }
    else {
      ImGui::PushStyleColor(ImGuiCol_Border, valueBackgroundActive);
      ImGui::PushStyleColor(ImGuiCol_Button, valueBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackground);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackground);
    }
    char keyId[256]; snprintf(keyId, 256, "%s ## button %s", name? name: "button", path);
    ImGui::Button(keyId, ImVec2(300, 100-OUTER_PADDING*2));
    if(ImGui::IsItemActive()){
      if(!buttonDown){
        buttonDown=strdup(keyId);
        char* uid=object_property(user, path);
        invoke_single_set(uid, (char*)"state", (char*)"down");
      }
    }
    else{
      if(buttonDown && !strcmp(buttonDown, keyId)){
        free(buttonDown); buttonDown=0;
        char* uid=object_property(user, path);
        invoke_single_set(uid, (char*)"state", (char*)"up");
      }
    }
    ImGui::PopStyleColor(4);
  }
  ImGui::SameLine();
  ImGui::EndChild();
  ImGui::PopStyleColor(2);
}

void draw_display(char* path, int16_t width, int16_t height)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(OUTER_PADDING,OUTER_PADDING));
  uint16_t ln = object_property_length(user, path);
  size_t l=strlen(path);
  int j; for(j=1; j<=ln; j++){
    if(!is_uid(object_property_get_n(user, path, j))) continue;
    snprintf(path+l, 128-l, ":%d", j);
    char pathis[128]; snprintf(pathis, 128, "%s:is", path);
    if(object_property_contains(user, pathis, (char*)"light")) draw_light(path, width);
    if(object_property_contains(user, pathis, (char*)"button")) draw_button(path, width);
    path[l] = 0;
  }
  ImGui::PopStyleVar();
}

