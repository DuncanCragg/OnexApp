
#include <imgui.h>
extern void ImStrncpy(char* dst, const char* src, size_t count);
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "im-gui.h"
#include "calendar.h"
#include "ux-features.h"

#pragma GCC diagnostic ignored "-Wformat-truncation"

#define DRAG_THRESHOLD         30.0f
#define START_DRIFT_THRESHOLD  10.0f
#define END_DRIFT_THRESHOLD     0.01f
#define DRIFT_DAMPING           0.5f

static ImVec2 startpoint(0,0);
char* dragPathId=0;
static float delta_x=0.0f;
static float delta_y=0.0f;
static bool  drag_handled = true;
static float drift_threshold = START_DRIFT_THRESHOLD;

#define MOVING_DELTA(x,y,d) (((x)*(x)+(y)*(y)) >= (d))

void kill_drag()
{
  free(dragPathId);
  dragPathId=0;
  delta_x = 0.0f;
  delta_y = 0.0f;
  drag_handled=true;
  drift_threshold = START_DRIFT_THRESHOLD;
  startpoint=ImVec2(0,0);
}

void track_drag(char* pathId, bool twodimensions)
{
  if(ImGui::IsItemActive() && !ImGui::IsMouseDragging() && dragPathId && strcmp(pathId, dragPathId)){
    kill_drag();
  }
  else
  if(ImGui::IsItemActive() && ImGui::IsMouseDragging()){
    if(!dragPathId){
      if(!startpoint.x && !startpoint.y) startpoint=ImGui::GetIO().MousePos;
      ImVec2 mp=ImGui::GetIO().MousePos;
      float dx=mp.x-startpoint.x;
      float dy=mp.y-startpoint.y;
      if(!twodimensions){ dx=0; dy*=0.5; }
      float distancemoved=sqrtf(dx*dx+dy*dy);
      if(distancemoved > DRAG_THRESHOLD) dragPathId=strdup(pathId);
    }
    else{
      ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
      delta_x=mouse_delta.x;
      delta_y=mouse_delta.y;
      drag_handled=false;
    }
  }
  else
  if(!ImGui::IsMouseDown(0) && MOVING_DELTA(delta_x, delta_y, drift_threshold) && dragPathId && !strcmp(pathId, dragPathId)){
    int msperframe = (int)(1000.0f/ImGui::GetIO().Framerate);
    delta_x *= (1.0f-(DRIFT_DAMPING*msperframe)/100);
    delta_y *= (1.0f-(DRIFT_DAMPING*msperframe)/100);
    drag_handled=false;
    drift_threshold = END_DRIFT_THRESHOLD;
  }
  else
  if(!ImGui::IsMouseDragging() && dragPathId && !strcmp(pathId, dragPathId)){
    kill_drag();
  }
}

void set_drag_scroll(char* path)
{
  if(!dragPathId) return;
  char* dragPathPath=strstr(dragPathId, "viewing-l");
  if(!dragPathPath) dragPathPath=strstr(dragPathId, "viewing-r");
  if(!dragPathPath) return;
  if(!strncmp(dragPathPath, path, strlen(path)) && strcmp(dragPathPath, path) && !drag_handled && MOVING_DELTA(delta_x,delta_y,0.1f)){
    ImGui::SetScrollX(ImGui::GetScrollX() - delta_x);
    ImGui::SetScrollY(ImGui::GetScrollY() - delta_y);
    drag_handled=true;
  }
}




#define LINK_THRESHOLD 20.0f
#define LINK_FROM 1
#define LINK_TO   2

int   linkDirection=0;
char* linkFrom=0;
char* linkTo=0;
ImVec2 linkToPos=ImVec2(0,0);
ImVec2 linkFromPos=ImVec2(0,0);

void best_prop_name(char* newpropname, int proplen, object* from, char* touid)
{
  if(object_property_contains(from, (char*)"is", (char*)"list")){
    ImStrncpy(newpropname, "list", proplen);
  }
  else{
    object* to=onex_get_from_cache(touid);
    char* is=object_property_values(to, (char*)"is");
    ImStrncpy(newpropname, is, proplen);
    for(char* p=newpropname; *p; p++) if(*p==' ') *p='-';
  }
}

void make_link()
{
  if(linkTo && linkFrom){
    char* lastcolon=strrchr(linkFrom,':');
    char* fromuid=0;
    char* propname;
    if(lastcolon){
      propname=lastcolon+1;
      int n; if(!sscanf(propname, "%u", &n)){
        *lastcolon=0;
        fromuid=object_property(user, linkFrom);
        *lastcolon=':';
      }
    }
    if(!fromuid){
      fromuid=object_property(user, linkFrom);
      propname=0;
    }
    object* objectEditing = onex_get_from_cache(fromuid);
    char* touid=object_property(user, linkTo);
    if(objectEditing && touid){
      char newpropname[128];
      if(!propname){ best_prop_name(newpropname, IM_ARRAYSIZE(newpropname), objectEditing, touid); propname=newpropname; }
      if(object_property_is(objectEditing, propname, (char*)"--")){
        invoke_single_set(fromuid, propname, touid);
      }
      else invoke_single_add(fromuid, propname, touid);
    }
  }
  if(linkFrom) free(linkFrom); linkFrom=0;
  if(linkTo) free(linkTo); linkTo=0;
  linkToPos=ImVec2(0,0);
  linkFromPos=ImVec2(0,0);
  linkDirection=0;
}

void track_link(bool from, char* path, int width, int height)
{
  if(ImGui::IsItemActive() && ImGui::IsMouseDragging() && !dragPathId){
    char*&  linkEnd   =from? linkFrom: linkTo;
    ImVec2& linkEndPos=from? linkFromPos: linkToPos;
    ImVec2 mp=ImGui::GetIO().MousePos;
    if(!linkEndPos.x && !linkEndPos.y) linkEndPos=mp;
    if(!linkEnd){
      float dx=linkEndPos.x-mp.x;
      float dy=linkEndPos.y-mp.y;
      dx=dx>=0? dx: -dx;
      dy=dy>=0? dy: -dy;
      if(dx>LINK_THRESHOLD && dy<LINK_THRESHOLD){
        linkEnd=strdup(path);
        linkDirection=from? LINK_FROM: LINK_TO;
        kill_drag();
      }
    }
  }
  else
  if(linkDirection && ImGui::IsMouseDragging() && !dragPathId){
    char*&  linkEnd=(linkDirection==LINK_TO? linkFrom: linkTo);
    ImVec2 cp=ImGui::GetCursorScreenPos();
    ImVec2 mp=ImGui::GetIO().MousePos;
    if(mp.x>cp.x-width && mp.x<cp.x && mp.y>cp.y && mp.y<cp.y+height){
      if(linkEnd && strcmp(linkEnd, path)){ free(linkEnd); linkEnd=0; }
      if(!linkEnd) linkEnd=strdup(path);
    }
    else{
      if(linkEnd && !strcmp(linkEnd, path)){ free(linkEnd); linkEnd=0; }
    }
  }
  else
  if(!ImGui::IsMouseDown(0) && !dragPathId){
    make_link();
  }
}

void draw_link()
{
  if(linkTo || linkFrom){
    ImVec2 startpos(0,0);
    ImGui::SetCursorScreenPos(startpos);
    ImVec4 transparent(0.00f, 0.00f, 0.00f, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, transparent);
    ImGui::BeginChild("Overlay", ImVec2(workspace1Width+workspace2Width, workspace1Height+buttonHeight), true);
    {
      ImVec2 mp=ImGui::GetIO().MousePos;
      ImVec2 to, from;
      if(linkToPos.x && linkToPos.y){     to=linkToPos; from=mp; }
      if(linkFromPos.x && linkFromPos.y){ to=mp;        from=linkFromPos; }
      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      draw_list->PushClipRectFullScreen();
      float arrowAngle=0.38;
      float arrowLength=25;
      float dx=to.x-from.x;
      float dy=to.y-from.y;
      float dv=sqrtf(dx*dx+dy*dy);
      dx=arrowLength*dx/dv; dy=arrowLength*dy/dv; dv=arrowLength;
      float dv2=dv/cos(arrowAngle);
      float a=acos(dy/dv);
      float dx2=dv2*sin(a-arrowAngle);
      float dy2=dv2*cos(a-arrowAngle);
      float dx3=dv2*sin(a+arrowAngle);
      float dy3=dv2*cos(a+arrowAngle);
      ImVec2 t1(dx>0? to.x-dx3: to.x+dx3, to.y-dy3);
      ImVec2 t2(dx>0? to.x-dx2: to.x+dx2, to.y-dy2);
      draw_list->AddCircleFilled(from, 10.0f, ImColor(actionColour));
      draw_list->AddTriangleFilled(to, t1, t2, ImColor(actionColour));
      draw_list->AddLine(to, from, ImColor(actionColour), 4.0f);
      draw_list->PopClipRect();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
  }
}




#define MAX_OPEN 64
static char* open[MAX_OPEN];

bool is_open(char* path)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strcmp(open[i], path)) return true;
  }
  return false;
}

void toggle_open(char* path)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strcmp(open[i], path)){ free(open[i]); open[i]=0; return; }
  }
  for(int i=0; i<MAX_OPEN; i++){
    if(!open[i]){ open[i]=strdup(path); return; }
  }
}

void close_all_starting(char* prefix)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strncmp(open[i], prefix, strlen(prefix))){ free(open[i]); open[i]=0; }
  }
}



// ---------------------------------------------------------------------------------------------

#define xTEST_ANDROID_KEYBOARD

extern "C" void showOrHideSoftKeyboard(bool show);

void show_keyboard(float multy)
{
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  if(yOffsetTarget) return;
  yOffsetTarget=(multy!=0)? multy: (ImGui::GetCursorScreenPos().y-3*buttonHeight)*0.80;
  yOffsetCounter=100;
  showOrHideSoftKeyboard(true);
#endif
}

void hide_keyboard()
{
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  yOffsetTarget=0;
  yOffsetCounter=0;
  yOffset=0;
  showOrHideSoftKeyboard(false);
#endif
}


static int ss= -1;
static int se= -1;

static int filter_and_autocomplete(ImGuiTextEditCallbackData* data, bool (*enforcer)(ImGuiTextEditCallbackData* data), char** autoCompleteChoices, int autoCompleteChoicesSize)
{
  static bool autocompletenext=false;
  if(data->EventFlag==ImGuiInputTextFlags_CallbackCharFilter){
    autocompletenext=true;
    return enforcer ? (enforcer(data)? 0: 1) : 0;
  }
  if(data->EventFlag==ImGuiInputTextFlags_CallbackAlways && autocompletenext){
    autocompletenext=false;
    int p=data->BufTextLen;
    if(p){
      int i;
      for(i=0; i<autoCompleteChoicesSize; i++){
        if(!strncasecmp(data->Buf, autoCompleteChoices[i], p)){
          data->DeleteChars(0, p);
          data->InsertChars(0, autoCompleteChoices[i]);
          ss=p;
          se=strlen(autoCompleteChoices[i]);
          data->BufDirty=true;
          break;
        }
      }
      if(i==autoCompleteChoicesSize){
        ss=-1;
        se=-1;
      }
    }
  }
  if(data->EventFlag==ImGuiInputTextFlags_CallbackAlways){
    if(ss!=-1) data->CursorPos=ss;
    if(ss!=-1) data->SelectionStart=ss;
    if(se!=-1) data->SelectionEnd=se;
    return 0;
  }
  return 0;
}

bool filter_auto_input_text(const char* id, char* buf, int buflen, ImGuiTextEditCallback fafn)
{
  int flags=ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_CallbackAlways|ImGuiInputTextFlags_EnterReturnsTrue;
  bool done=ImGui::InputText(id, buf, buflen, flags, fafn, buf);
  if(done){ ss= -1; se= -1; }
  return done;
}

static bool enforcePropertyName(ImGuiTextEditCallbackData* data)
{
  ImWchar ch = data->EventChar;
  if(ch >=256) return false;
  if(!strlen((char*)(data->UserData)) && !isalpha(ch)) return false;
  if(ch == ' '){ data->EventChar = '-'; return true; }
  if(ch == '-') return true;
  if(!isalnum(ch)) return false;
  data->EventChar = tolower(ch);
  return true;
}

int filter_and_autocomplete_calendar_tags(ImGuiTextEditCallbackData* data)
{
  int tls=object_property_size(config, (char*)"taglookup");
  char* calendarTags[tls-1];
  for(int t=2; t<=tls; t++){
    calendarTags[t-2]=object_property_key(config, (char*)"taglookup", t);
  }
  return filter_and_autocomplete(data, enforcePropertyName, calendarTags, tls-1);
}

int filter_and_autocomplete_default(ImGuiTextEditCallbackData* data)
{
  return filter_and_autocomplete(data, 0, 0, 0);
}

static const char* propertyNameChoices[] = {
  "is",
  "title",
  "description",
  "text",
  "name",
  "list",
  "date",
  "time",
  "end-time",
  "end-date",
  "every",
  "tags",
  "cost",
  "total",
  "Rules",
  "Timer",
  "Notifying"
};

int filter_and_autocomplete_property_names(ImGuiTextEditCallbackData* data)
{
  return filter_and_autocomplete(data, enforcePropertyName, (char**)propertyNameChoices, IM_ARRAYSIZE(propertyNameChoices));
}
