
extern "C" {
#include <items.h>
}

#include <imgui.h>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#include "im-gui.h"
#include "ux-features.h"
#include "calendar.h"

#pragma GCC diagnostic ignored "-Wformat-truncation"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#define UPPER_SCROLL_JUMP 20
#define COLUMN_WIDTH 225

static bool firstDateSet=false;

properties* calstamps=0;
char* calendarUIDs[16];
time_t firstDate=0;
time_t lastDate=0;

static void get_cell_events_and_show_open(char* path, struct tm* thisdate, int col);
static void get_cell_titles(char* titles, struct tm* thisdate, int col);
static void get_tag_icons(char* tagicons, int taglen, struct tm* thisdate, int cols);
static void draw_day_cell(char* path, struct tm* thisdate, int day, int col, int16_t width);
static void save_days(char* path);

extern "C" void showNotification(char* title, char* text);
extern "C" void setAlarm(time_t when, char* uid);

const char* daytable[] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
const char* monthtable[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

char* calendarTitles[16];

time_t todayseconds = 0;
struct tm todaydate;

void set_time_save_days()
{
  todayseconds=time(0);
  todaydate = *localtime(&todayseconds);
  save_days((char*)"viewing-r");
}

int date_compare(struct tm* d1, struct tm* d2)
{
  int yd=(d1->tm_year - d2->tm_year); if(yd) return yd;
  int md=(d1->tm_mon  - d2->tm_mon);  if(md) return md;
  int dd=(d1->tm_mday - d2->tm_mday); if(dd) return dd;
  return 0;
}

static const char* get_hex_of_colour(char* colour)
{
  if(!colour) return "";
  if(!strcmp(colour, "red"    )) return "\033\xff\x01\x01\xff";
  if(!strcmp(colour, "yellow" )) return "\033\xff\xff\x01\xff";
  if(!strcmp(colour, "green"  )) return "\033\x01\xdf\x01\xff";
  if(!strcmp(colour, "cyan"   )) return "\033\x01\xff\xff\xff";
  if(!strcmp(colour, "blue"   )) return "\033\x01\x01\xff\xff";
  if(!strcmp(colour, "magenta")) return "\033\xff\x01\xff\xff";
  return "";
}

static const char* date_formats[] = {    "%d %b %I%p",  "%d %b %I%p",           //     23 Feb 7pm
                                         "%b %d %I%p",  "%b %d %I%p",           //     Feb 23 7pm
                                         "%d %b %I:%M%p",  "%d %b %I.%M%p",     //     23 Feb 7:15pm
                                         "%b %d %I:%M%p",  "%b %d %I.%M%p",     //     Feb 23 7:15pm
                                         "%d %b %H:%M",    "%d %b %H.%M",       //     23 Feb 19:00
                                         "%b %d %H:%M",    "%b %d %H.%M",       //     Feb 23 19:00
                                         "%d %b %Y",       "%d %b",             //     23 Feb (2019)
                                         "%b %d %Y",       "%b %d",             //     Feb 23 (2019)
                                      "%a %d %b %H:%M", "%a %d %b %H.%M",       // Mon 23 Feb 19:00
                                      "%a %b %d %H:%M", "%a %b %d %H.%M",       // Mon Feb 23 19:00
                                      "%a %d %b %Y",    "%a %d %b",             // Mon 23 Feb (2019)
                                      "%a %b %d %Y",    "%a %b %d",             // Mon Feb 23 (2019)
                                      "%Y-%m-%d", "%Y/%m/%d",                   //  2019-02-23 2019/02/23
                                      "%I:%M%p", "%I.%M%p", "%I%p",             //  7:00pm 7pm
                                      "%H:%M", "%H.%M",                         //  19:00
                                      "%Y-%m-%dT%H:%M:%S", "%Y-%m-%d %H:%M:%S"  //  2019-2-23T19:00:11, 2019-02-23 19:00:11
};

static const char* time_formats[] = { "%I:%M%p", "%I.%M%p", "%I%p", //  7:00pm 7pm
                                      "%H:%M", "%H.%M",             //  19:00
};

bool get_date(char* p, struct tm* parsed_date)
{
  for(int f=0; f<IM_ARRAYSIZE(date_formats); f++){
    memset(parsed_date, 0, sizeof(struct tm));
    (*parsed_date).tm_mday=todaydate.tm_mday;
    (*parsed_date).tm_mon =todaydate.tm_mon;
    (*parsed_date).tm_year=todaydate.tm_year;
    char* q=strptime(p, date_formats[f], parsed_date);
    if(q) return true;
  }
  return false;
}

bool get_time(char** p, struct tm* parsed_time)
{
  for(int f=0; f<IM_ARRAYSIZE(time_formats); f++){
    memset(parsed_time, 0, sizeof(struct tm));
    char* q=strptime(*p, time_formats[f], parsed_time);
    if(q){ *p=q; return true; }
  }
  return false;
}

time_t get_date_from_object(object* o, char* path, struct tm* parsed_date)
{
  char* p=object_property_values(o, path);
  if(!p) return -1;
  if(!get_date(p, parsed_date)) return -1;
  return mktime(parsed_date);
}

void save_day(char* path, int j, int col)
{
  char stpath[128]; snprintf(stpath, 128, "%s:%d:date", path, j);
  struct tm start_date;
  time_t t=get_date_from_object(user, stpath, &start_date);
  if(t== -1) return;
  if(!firstDateSet){
    if(firstDate==0 || t<firstDate) firstDate=t;
    if(lastDate==0  || t>lastDate)  lastDate=t;
  }
  char ts[32]; int n=strftime(ts, 32, "%Y-%m-%d", &start_date);
  snprintf(ts+n, 32-n, "/%d", col);
  char eventpath[128]; snprintf(eventpath, 128, "%s:%d", path, j);
  list* l=(list*)properties_get(calstamps, value_new(ts));
  if(!l) l=list_new(32);
  list_add(l, value_new(eventpath));
  properties_set(calstamps, value_new(ts), l);
}

void save_days(char* path)
{
  if(!calstamps) calstamps=properties_new(100);
  else           properties_clear(calstamps, true);
  uint16_t ln = object_property_length(user, path);
  int col=1;
  for(int c=1; c<16; c++){ calendarTitles[c]=0; calendarUIDs[c]=0; }
  int j; for(j=1; j<=ln; j++){
    char* val=object_property_get_n(user, path, j);
    if(!is_uid(val)) continue;
    char calpath[128]; snprintf(calpath, 128, "%s:%d", path, j);
    char ispath[128]; snprintf(ispath, 128, "%s:is", calpath);
    if(!object_property_contains(user, ispath, (char*)"event")) continue;
    if(object_property_contains(user, ispath, (char*)"list")){
      char listpath[128]; snprintf(listpath, 128, "%s:list", calpath);
      uint16_t ln2 = object_property_length(user, listpath);
      int k; for(k=1; k<=ln2; k++){
        char* val=object_property_get_n(user, listpath, k);
        if(!is_uid(val)) continue;
        char ispath[128]; snprintf(ispath, 128, "%s:%d:is", listpath, k);
        if(!object_property_contains(user, ispath, (char*)"event")) continue;
        save_day(listpath, k, col);
      }
      char titlepath[128]; snprintf(titlepath, 128, "%s:title", calpath);
      char* caltitle=object_property_values(user, titlepath);
      calendarTitles[col]=caltitle;
      char* caluid=object_property(user, calpath);
      calendarUIDs[col]=caluid;
      col++;
    }
  }
  if(col==1) for(j=1; j<=ln; j++){
    char* val=object_property_get_n(user, path, j);
    if(!is_uid(val)) continue;
    char calpath[128]; snprintf(calpath, 128, "%s:%d", path, j);
    char ispath[128]; snprintf(ispath, 128, "%s:is", calpath);
    if(!object_property_contains(user, ispath, (char*)"event")) continue;
    if(!object_property_contains(user, ispath, (char*)"list")){
      save_day(path, j, col);
    }
  }
  if(!firstDate) firstDate=todayseconds;
  firstDateSet=true;
}

static bool same_day(struct tm* d1, struct tm* d2)
{
  return (*d1).tm_mday==(*d2).tm_mday &&
         (*d1).tm_mon ==(*d2).tm_mon &&
         (*d1).tm_year==(*d2).tm_year;
}

void draw_calendar(char* path, int16_t width, int16_t height)
{
  static int firstdaydelta=0;
  static float scrollx=0;
  static float scrolly=0;
  int lastday=UPPER_SCROLL_JUMP*2+(int)((scrolly+40.0f)/(2*buttonHeight));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
  ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, listBackground);
  ImGui::PushStyleColor(ImGuiCol_Text, renderColour);
  ImGui::PushStyleColor(ImGuiCol_Button, renderBackground);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, renderBackground);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, renderBackgroundActive);

  ImGui::BeginGroup();

  int daysabove=(int)((scrolly+40.0f)/(2*buttonHeight));
  time_t daystamp = firstDate-(firstdaydelta-daysabove)*(24*60*60);
  struct tm thisdate = *localtime(&daystamp);
  bool jumpToToday=false;
  char tplId[256]; snprintf(tplId, 256, "%s\n%d##topleft cell %s:", monthtable[thisdate.tm_mon], thisdate.tm_year+1900, path);
  if(ImGui::Button(tplId, ImVec2(COLUMN_WIDTH, buttonHeight*2)) && !dragPathId){
    jumpToToday=true;
  }
  track_drag(tplId, true);

  char datecol[32]; snprintf(datecol, 32, "datecol");
  ImGui::BeginChild(datecol, ImVec2(COLUMN_WIDTH,height-2*buttonHeight), true);
  {
    time_t daystamp = firstDate-firstdaydelta*(24*60*60);
    for(int day=0; day< lastday; day++, daystamp+=(24*60*60)){
      struct tm thisdate = *localtime(&daystamp);
      if(thisdate.tm_wday==0 || thisdate.tm_wday==6){
        ImGui::PushStyleColor(ImGuiCol_Text, renderColour);
        ImGui::PushStyleColor(ImGuiCol_Button, valueBackground);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackground);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackgroundActive);
      }
      if(same_day(&thisdate, &todaydate) && thisdate.tm_mday==1){
        ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
      }
      else
      if(same_day(&thisdate, &todaydate)){
        ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
      }
      else
      if(thisdate.tm_mday==1){
        ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
        ImGui::PushStyleColor(ImGuiCol_Button, renderBackground);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, renderBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, renderBackgroundActive);
      }

      char tagicons[512]=""; get_tag_icons(tagicons, 512, &thisdate, 4);
      char dayId[256];
      snprintf(dayId, 256, "%s %d\n%s%s## %d %s:", daytable[thisdate.tm_wday], thisdate.tm_mday, (thisdate.tm_mday==1 || day==0)? monthtable[thisdate.tm_mon]: "", tagicons, day, path);
      ImGui::Button(dayId, ImVec2(COLUMN_WIDTH, buttonHeight*2));
      track_drag(dayId, true);

      if(same_day(&thisdate, &todaydate) || thisdate.tm_mday==1) ImGui::PopStyleColor(4);
      if(thisdate.tm_wday==0 || thisdate.tm_wday==6)            ImGui::PopStyleColor(4);
    }
  }
  ImGui::EndChild();

  ImGui::EndGroup();

  ImGui::SameLine();

  ImGui::BeginGroup();

  char calrow[32]; snprintf(calrow, 32, "calrow");
  ImGui::SetNextWindowContentSize(ImVec2(width*2.02f, 0.0f));
  ImGui::BeginChild(calrow, ImVec2(width,2*buttonHeight), true);
  {
    for(int col=1; col<=4; col++){
      if(col>1) ImGui::SameLine();
      char colId[256]; snprintf(colId, 256, "%s##calendarTitles[%d] %s:", calendarTitles[col]? calendarTitles[col]: (char*)"", col, path);
      ImGui::Button(colId, ImVec2(2*COLUMN_WIDTH, buttonHeight*2));
      track_drag(colId, true);
    }
  }
  ImGui::EndChild();

  char calbody[32]; snprintf(calbody, 32, "calbody");
  ImGui::SetNextWindowContentSize(ImVec2(width*2.02f, 0.0f));
  ImGui::BeginChild(calbody, ImVec2(width,height-2*buttonHeight), true);
  {
    time_t daystamp = firstDate-firstdaydelta*(24*60*60);
    for(int day=0; day< lastday; day++, daystamp+=(24*60*60)){
      struct tm thisdate = *localtime(&daystamp);
      if(thisdate.tm_wday==0 || thisdate.tm_wday==6){
        ImGui::PushStyleColor(ImGuiCol_Text, renderColour);
        ImGui::PushStyleColor(ImGuiCol_Button, valueBackground);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueBackground);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueBackgroundActive);
      }
      for(int col=1; col<=4; col++){
        if(col>1) ImGui::SameLine();
        draw_day_cell(path, &thisdate, day, col, width);
      }
      if(thisdate.tm_wday==0 || thisdate.tm_wday==6) ImGui::PopStyleColor(4);
    }
  }
  ImGui::EndChild();

  ImGui::EndGroup();

  ImGui::BeginChild(calbody);
  set_drag_scroll(path);
  scrollx=ImGui::GetScrollX();
  scrolly=ImGui::GetScrollY();
  if(jumpToToday){
    scrolly=UPPER_SCROLL_JUMP*buttonHeight*2;
    ImGui::SetScrollY(scrolly);
    firstdaydelta=(firstDate-todayseconds)/(24*60*60)+UPPER_SCROLL_JUMP+2;
  }
  else
  if(scrolly<UPPER_SCROLL_JUMP*buttonHeight*2 && !dragPathId){
    scrolly+=UPPER_SCROLL_JUMP*buttonHeight*2;
    ImGui::SetScrollY(scrolly);
    firstdaydelta+=UPPER_SCROLL_JUMP;
    kill_drag();
  }
  ImGui::EndChild();

  ImGui::BeginChild(datecol);
  ImGui::SetScrollY(scrolly);
  ImGui::EndChild();

  ImGui::BeginChild(calrow);
  ImGui::SetScrollX(scrollx);
  ImGui::EndChild();

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar();
}

void ensure_alarm(object* o, time_t t)
{
  setAlarm(t, object_property(o, (char*)"UID"));
  object_keep_active(o, true);
}

void cancel_alarm(object* o)
{
  setAlarm(0, object_property(o, (char*)"UID"));
  object_keep_active(o, false);
}

bool evaluate_event(object* o, void* d)
{
  log_write("evaluate_event\n"); object_log(o);
  if(!object_property_contains(o, (char*)"is", (char*)"event")){   log_write("object is no longer an is: event\n");
    cancel_alarm(o);
    object_set_evaluator(o, (char*)"default");
    return true;
  }
  struct tm date; time_t t=get_date_from_object(o, (char*)"date", &date);
  todayseconds=time(0); todaydate = *localtime(&todayseconds);
  if(t== -1) date=todaydate;
  if(date_compare(&date, &todaydate) < 0){    log_write("event for past date, ignored\n");
    cancel_alarm(o);
    return true;
  }
  log_write("event for today or future\n");
  char* ts=object_property_values(o, (char*)"time");
  struct tm time;
  if(ts && get_time(&ts, &time)){
    date.tm_sec =time.tm_sec;
    date.tm_min =time.tm_min;
    date.tm_hour=time.tm_hour;
    t=mktime(&date);
  }
  if(t<=todayseconds+3){
    char* title=object_property_values(o, (char*)"title");
    char* text=(char*)"!!";
    cancel_alarm(o);
    showNotification(title, text);
  }
  else{
    ensure_alarm(o, t);
  }
  return true;
}

typedef struct {
  char* title;
  char* date;
  char* time;
  char* endtime;
  char* tags;
} eventinit;

object* create_new_event(struct tm* thisdate, char* title)
{
  object* r=object_new(0, (char*)"event", (char*)"event", 8);
  char ts[32]; strftime(ts, 32, "%Y-%m-%d", thisdate);
  object_property_set(r, (char*)"title", title);
  object_property_set(r, (char*)"date", ts);
  char* time=0;
  char* endtime=0;
  char* p=title;
  for(;*p;p++){
    if(isdigit(*p)){
      struct tm parsed_time;
      if(get_time(&p, &parsed_time)){
        char ts[32]; int n=strftime(ts, 32, "%I:%M%P", &parsed_time);
        char* tsd=strdup(ts);
        if(!time) time=tsd;
        else
        if(!endtime) endtime=tsd;
        if(time && endtime) break;
      }
    }
  }
  char tags[256]=""; int l=0;
  int tls=object_property_size(config, (char*)"taglookup");
  for(int t=2; t<=tls; t++){
    char* tag=object_property_key(config, (char*)"taglookup", t);
    char* val=object_property_val(config, (char*)"taglookup", t);
    if(strcasestr(title, tag)) l+=snprintf(tags+l, 256-l, "%s ", val);
  }
  if(time){     object_property_set(r, (char*)"time",     time);     free(time); }
  if(endtime){  object_property_set(r, (char*)"end-time", endtime);  free(endtime); }
  if(*tags) object_property_set(r, (char*)"tags", tags);
  else      object_property_set(r, (char*)"tags", (char*)"--");
  object_keep_active(r, true);
  return r;
}

void draw_day_cell(char* path, struct tm* thisdate, int day, int col, int16_t width)
{
  static char dayBuf[256] = "";
  static char* editingCell=0;
  static bool grabbedFocus=false;

  bool canAdd=(col==1 || calendarTitles[col]);
  char addId[256]; snprintf(addId, 256, canAdd? " +##%d %d %s:": "##%d %d %s:", day, col, path);
  bool editing = editingCell && !strcmp(addId, editingCell);
  ImGuiIO& io = ImGui::GetIO();
  if(editing && grabbedFocus && (!io.WantTextInput || keyboardCancelled)){
    hide_keyboard();
    free(editingCell); editingCell=0;
    *dayBuf=0;
    grabbedFocus=false;
    editing=false;
  }
  char titles[512]="";
  if(!editing){
    get_cell_titles(titles, thisdate, col);
    if(*titles){
      char evtId[256]; snprintf(evtId, 256, "%s##%d %d %s:", titles, day, col, path);
      if(ImGui::Button(evtId, ImVec2(2*COLUMN_WIDTH-smallButtonWidth, buttonHeight*2)) && !dragPathId){
        get_cell_events_and_show_open(path, thisdate, col);
        calendarView=false;
      }
      track_drag(evtId, true);
    }
  }else{
    if(!grabbedFocus){
      ImGui::SetKeyboardFocusHere();
      grabbedFocus = io.WantTextInput;
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, valueBackground);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, valueBackgroundActive);
    int flags=ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CtrlEnterForNewLine|ImGuiInputTextFlags_AutoSelectAll;
    char valId[256]; snprintf(valId, 256, "## day cell %s:", path);
    if(ImGui::InputTextMultiline(valId, dayBuf, IM_ARRAYSIZE(dayBuf), ImVec2(2*COLUMN_WIDTH, buttonHeight*2), flags)){
      if(*dayBuf){
        object* o=create_new_event(thisdate, dayBuf);
        if(o){
          char* evtuid=object_property(o, (char*)"UID");
          char* caluid=calendarUIDs[col];
          if(caluid){
            invoke_single_add(caluid, (char*)"list", evtuid);
          }
          else{
            object_property_add(user, path, evtuid);
          }
          set_time_save_days();
        }
        hide_keyboard();
        free(editingCell); editingCell=0;
        *dayBuf=0;
        grabbedFocus=false;
      }
    }
    ImGui::PopStyleColor(2);
  }
  if(!editing){
    if(*titles) ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, renderColourSoft);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f,0.5f));
    if(ImGui::Button(addId, ImVec2(*titles? smallButtonWidth: 2*COLUMN_WIDTH, buttonHeight*2)) && canAdd && !editing && !dragPathId){
      editingCell=strdup(addId);
      show_keyboard(0);
    }
    track_drag(addId, true);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  }
}

void get_cell_titles(char* titles, struct tm* thisdate, int col)
{
  char ts[32]; int n=strftime(ts, 32, "%Y-%m-%d", thisdate);
  snprintf(ts+n, 32-n, "/%d", col);
  list* l=(list*)properties_get(calstamps, value_new(ts));
  if(l){
    int at=0;
    static properties* uidseen=properties_new(100);
    properties_clear(uidseen, false);
    for(int e=1; e<=list_size(l); e++){
      char* eventpath=value_string((value*)list_get_n(l,e));
      char* eventuid=object_property(user, eventpath);
      if(!properties_get(uidseen, value_new(eventuid))){
        properties_set(uidseen, value_new(eventuid), value_new(eventuid));
        char titlepath[128]; snprintf(titlepath, 128, "%s:title", eventpath);
        char* title=object_property_values(user, titlepath);
        at+=snprintf(titles+at, 512-at, "%s\n", title? title: (char*)"--");
      }
    }
  }
}

void get_tag_icons(char* tagicons, int taglen, struct tm* thisdate, int cols)
{
  int ti=0;
  static properties* uidseen=properties_new(100);
  properties_clear(uidseen, false);
  for(int col=1; col<=cols; col++){
    char ts[32]; int n=strftime(ts, 32, "%Y-%m-%d", thisdate);
    snprintf(ts+n, 32-n, "/%d", col);
    list* l=(list*)properties_get(calstamps, value_new(ts));
    if(!l) continue;
    for(int e=1; e<=list_size(l); e++){
      char* eventpath=value_string((value*)list_get_n(l,e));
      char* eventuid=object_property(user, eventpath);
      if(!properties_get(uidseen, value_new(eventuid))){
        properties_set(uidseen, value_new(eventuid), value_new(eventuid));
        char tagpath[128];
        snprintf(tagpath, 128, "%s:tags", eventpath);
        int ln=object_property_length(user, tagpath);
        for(int i=1; i<=ln; i++){
          size_t l=strlen(tagpath);
          snprintf(tagpath+l, 128-l, ":%d:icon", i);
          char* icon=object_property_values(user, tagpath);
          tagpath[l] = 0;
          snprintf(tagpath+l, 128-l, ":%d:colour", i);
          char* colour=object_property_values(user, tagpath);
          tagpath[l] = 0;
          if(icon) ti+=snprintf(tagicons+ti, taglen-ti, "%s%s ", get_hex_of_colour(colour), icon);
        }
      }
    }
  }
}

void get_cell_events_and_show_open(char* path, struct tm* thisdate, int col)
{
  char ts[32]; int n=strftime(ts, 32, "%Y-%m-%d", thisdate);
  snprintf(ts+n, 32-n, "/%d", col);
  list* l=(list*)properties_get(calstamps, value_new(ts));
  if(l){
    for(int e=1; e<=list_size(l); e++){
      char* eventpath=value_string((value*)list_get_n(l,e));
      char* openuid=object_property(user, eventpath);
      uint16_t ln = object_property_length(user, path);
      int i; for(i=1; i<=ln; i++){
        char* uid=object_property_get_n(user, path, i);
        if(uid && openuid && !strcmp(uid, openuid)) break;
      }
      if(i==ln+1) object_property_add(user, path, openuid);
      char openPath[256]; snprintf(openPath, 256, "%s:%d", path, i);
      if(!is_open(openPath)) toggle_open(openPath);
    }
  }
}


