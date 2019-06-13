
static ImGuiWindowFlags window_flags = 0;
static unsigned char* fontData;
static int texWidth, texHeight;

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

ImVec4 schemeBrown(183.0f/255, 142.0f/255, 96.0f/255, 1.0f);
ImVec4 schemeYellow(255.0f/255, 245.0f/255, 180.0f/255, 1.0f);
ImVec4 schemeMauve(221.0f/255, 190.0f/255, 243.0f/255, 1.0f);
ImVec4 schemePurple(169.0f/255, 103.0f/255, 212.0f/255, 1.0f);
ImVec4 schemeGreen(160.0f/255, 175.0f/255, 110.0f/255, 1.0f);
ImVec4 schemeLightPurple(0.8f, 0.7f, 0.9f, 1.0f);
ImVec4 schemeDarkerPurple(0.73f, 0.63f, 0.83f, 1.0f);
ImVec4 schemePlum(230.0f/255, 179.0f/255, 230.0f/255, 1.0f);

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

// ---------------------------------------------------------------------------------------------

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

void draw_view();
void draw_object_properties(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
void draw_new_property_value_editor(char* path, char* propname, char* val, bool single, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
void draw_padding(char* path, int16_t width, int16_t height, int8_t depth);
void draw_new_value_or_object_button(char* path, int16_t width, int j, int8_t depth, bool valueToo);

object* create_new_object_like_others(char* path);
object* create_new_object_for_property_name(char* path, char* name);
object* create_new_event(struct tm* thisdate, char* title);
int16_t calculate_scroller_height(char* path, int16_t height);
void get_summary(char* path, char* summary);
bool get_summary_from(char* path, char* summary, const char* key);
int16_t calculate_key_width(char* path);
void draw_object_header(char* path, bool locallyEditable, int16_t width, int8_t depth);
void draw_object_footer(char* path, bool locallyEditable, int16_t width, int16_t keyWidth, int8_t depth);
void draw_nested_object_properties_list(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth);
void draw_key(char* path, char* key, int16_t width, int16_t height, int16_t keyWidth, int8_t depth);
void draw_property_list(char* pathkey, char* key, bool locallyEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth);
void draw_calendar(char* path, int16_t width, int16_t height);
void draw_day_cell(char* path, struct tm* thisdate, int day, int col, int16_t width);
void get_cell_titles(char* titles, struct tm* thisdate, int col);
void get_tag_icons(char* tagicons, int taglen, struct tm* thisdate, int cols);
void get_cell_events_and_show_open(char* path, struct tm* thisdate, int col);
void save_days(char* path);
void save_day(char* path, int j, int col);
void make_link();
void best_prop_name(char* newpropname, int proplen, object* from, char* touid);
void draw_link();
void track_link(bool from, char* path, int width, int height);
char* pop_last(char* path);
void set_property_name(char* path , char* name);
void set_property_name_and_object(char* path , char* name);
void set_property_name_and_link(char* path , char* name);
void set_new_value(char* path, char* valBuf, bool single);
void set_new_tag(char* path, char* valBuf);
void hide_keyboard();
void show_keyboard(float multy);

static GUI* static_gui;

static object* user;
static object* config;

#define DARKEN_DEPTH 3

static uint16_t buttonHeight=70;
static uint16_t paddingHeight=15;
static uint16_t objectHeight=400;
static uint16_t listHeight=1000;

static uint16_t shorterValWidth=680;
static uint16_t buttonWidth=190;
static uint16_t smallButtonWidth=65;
static uint16_t rhsPadding=20;

static uint16_t workspace1Width;
static uint16_t workspace1Height;
static uint16_t workspace2Width;
static uint16_t workspace2Height;

uint16_t    yOffsetTarget=0;
uint16_t    yOffset=0;
uint16_t    yOffsetCounter=0;

static bool rhsFullScreen=false;
static bool calendarView=false;
static bool tableView=false;
#define MAX_OPEN 64
static char* open[MAX_OPEN];

#define DRAG_THRESHOLD         30.0f
#define START_DRIFT_THRESHOLD  10.0f
#define END_DRIFT_THRESHOLD     0.01f
#define DRIFT_DAMPING           0.5f

static ImVec2 startpoint(0,0);
static char* dragPathId=0;
static float delta_x=0.0f;
static float delta_y=0.0f;
static bool  drag_handled = true;
static float drift_threshold = START_DRIFT_THRESHOLD;

#define MOVING_DELTA(x,y,d) (((x)*(x)+(y)*(y)) >= (d))

static void kill_drag()
{
  free(dragPathId);
  dragPathId=0;
  delta_x = 0.0f;
  delta_y = 0.0f;
  drag_handled=true;
  drift_threshold = START_DRIFT_THRESHOLD;
  startpoint=ImVec2(0,0);
}

static void track_drag(char* pathId, bool twodimensions)
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

static void set_drag_scroll(char* path)
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

static int   linkDirection=0;
static char* linkFrom=0;
static char* linkTo=0;
static ImVec2 linkToPos=ImVec2(0,0);
static ImVec2 linkFromPos=ImVec2(0,0);

static bool is_open(char* path)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strcmp(open[i], path)) return true;
  }
  return false;
}

static void toggle_open(char* path)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strcmp(open[i], path)){ free(open[i]); open[i]=0; return; }
  }
  for(int i=0; i<MAX_OPEN; i++){
    if(!open[i]){ open[i]=strdup(path); return; }
  }
}

static time_t todayseconds = 0;
static struct tm todaydate;


static void close_all_starting(char* prefix)
{
  for(int i=0; i<MAX_OPEN; i++){
    if(open[i] && !strncmp(open[i], prefix, strlen(prefix))){ free(open[i]); open[i]=0; }
  }
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

static bool firstDateSet=false;

static const char* daytable[] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
static const char* monthtable[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

static properties* calstamps=0;
static time_t firstDate=0;
static time_t lastDate=0;
static char* calendarTitles[16];
static char* calendarUIDs[16];

#define UPPER_SCROLL_JUMP 20
#define COLUMN_WIDTH 225

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

void draw_object_header(char* path, bool locallyEditable, int16_t width, int8_t depth)
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
        object* objectEditing = onex_get_from_cache(object_property(user, path));
        *lastcolon=':'; *secondlastcolon=':';
        object_property_set(objectEditing, secondlastcolon+1, (char*)"");
      }
      else{
        object_property_set(user, path, (char*)"");
        if(static_gui) static_gui->changed();
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
    if(static_gui) static_gui->changed();
  }
  track_drag(pikId, true);

  ImGui::PopStyleColor(4);
  ImGui::PopStyleVar(1);
}

void draw_object_properties(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
{
  draw_object_header(path, locallyEditable, width, depth);
  if(strcmp(path, "viewing-l") && !is_open(path)) return;
  int16_t scrollerheight=calculate_scroller_height(path, height);
  int16_t keyWidth=calculate_key_width(path);
  int8_t sz = object_property_size(user, path);
  if(sz>0) for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, path, i);
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s:%s", path, key);
    if(!key) log_write("key=null: path=%s pathkey=%s i=%d sz=%d values: %s value: %s\n", path, pathkey, i, sz, object_property(user, pathkey), object_property_val(user, path, i));
    uint16_t ln = object_property_length(user, pathkey);
    uint32_t wid=0;
    for(int j=1; j<=ln; j++){
      char* val=object_property_get_n(user, pathkey, j);
      if(is_uid(val)){ wid=0; break; }
      wid += strlen(val)+1;
    }
    int hgt;
    if(wid >0) hgt=(wid/40+1)*buttonHeight;
    else       hgt=scrollerheight;
    if(hgt>=buttonHeight) draw_property_list(pathkey, key, locallyEditable, width, hgt, keyWidth, depth);
    else{
      char blnId[256]; snprintf(blnId, 256, "##filler %d %d %s", width, hgt, pathkey);
      ImGui::Button(blnId, ImVec2(width, paddingHeight));
      track_drag(blnId, true);
    }
  }
  draw_object_footer(path, locallyEditable, width, keyWidth, depth);
}

void draw_view()
{
  int msperframe = (int)(1000.0f/ImGui::GetIO().Framerate);
  if(yOffsetCounter > 0){
    yOffset=yOffsetTarget*(100-yOffsetCounter)/100;
    yOffsetCounter-=msperframe >10? 10 : 5;
  }
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
      bool locallyEditable = object_is_local(uid);
      draw_object_properties(path, locallyEditable, workspace1Width-rhsPadding, workspace1Height, 1);
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

    if(calendarView) ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    else             ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    if(ImGui::Button(" calendar", ImVec2(buttonWidth+smallButtonWidth, buttonHeight)))
    {
      calendarView=!calendarView;
      if(calendarView){
        tableView=false;
        close_all_starting((char*)"viewing-r");
      }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if(tableView) ImGui::PushStyleColor(ImGuiCol_Text, propertyColour);
    else          ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
    if(ImGui::Button(" table", ImVec2(buttonWidth, buttonHeight)))
    {
      tableView=!tableView;
      if(tableView){
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

    ImGui::Separator();

    int8_t s=strlen("viewing-r")+1; char path[s]; memcpy(path, "viewing-r", s);
    if(calendarView) draw_calendar(path, ws2width-rhsPadding, workspace2Height-100);
    else
    if(tableView);// drawTable(..);
    else             draw_nested_object_properties_list(path, false, ws2width-rhsPadding, workspace2Height-100, 1);
  }
  ImGui::EndChild();
  draw_link();
}

VulkanBase *app;

void drawGUI()
{
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2((float)app->width, (float)app->height), ImGuiSetCond_FirstUseEver);

  if(!ImGui::Begin("Onex", NULL, window_flags)){
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
//style.TouchExtraPadding = ImVec2(10.0f,10.0f);

  draw_view();

  ImGui::PopStyleVar(svs);

  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(650, 650), ImGuiSetCond_FirstUseEver);
//ImGui::ShowTestWindow();

  ImGui::Render();
}

// ---------------------------------------------------------------------------------------------

char*       propNameEditing=0;
bool        keyboardCancelled=false;

#define xTEST_ANDROID_KEYBOARD

void show_keyboard(float multy){
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  if(yOffsetTarget) return;
  yOffsetTarget=(multy!=0)? multy: (ImGui::GetCursorScreenPos().y-3*buttonHeight)*0.80;
  yOffsetCounter=100;
  showOrHideSoftKeyboard(true);
#endif
}

void hide_keyboard(){
#if defined(__ANDROID__) || defined(TEST_ANDROID_KEYBOARD)
  yOffsetTarget=0;
  yOffsetCounter=0;
  yOffset=0;
  showOrHideSoftKeyboard(false);
#endif
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
        object_property_set(objectEditing, propname, touid);
      }
      else object_property_add(objectEditing, propname, touid);
    }
  }
  if(linkFrom) free(linkFrom); linkFrom=0;
  if(linkTo) free(linkTo); linkTo=0;
  linkToPos=ImVec2(0,0);
  linkFromPos=ImVec2(0,0);
  linkDirection=0;
}

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

void set_new_tag(char* path, char* tag)
{
  char* lastcolon=strrchr(path,':'); *lastcolon=0;
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  if(objectEditing){
    if(*tag){
      char tagpath[64]; snprintf(tagpath, 64, "taglookup:%s", tag);
      char* tagUID=object_property(config, tagpath);
      if(!tagUID) return;
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        object_property_set(objectEditing, lastcolon+1, tagUID);
      }
      else object_property_add(objectEditing, lastcolon+1, tagUID);
    }
    else{
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        object_property_set(objectEditing, lastcolon+1, (char*)"");
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
    keyval kv = { lastcolon+1, buf };
    onex_run_evaluators(uid, &kv);
    *lastcolon=':';
  }
  else{
    char* lastcolon=strrchr(path,':'); *lastcolon=0;
    char* secondlastcolon=strrchr(path, ':'); *secondlastcolon=0;
    object* objectEditing = onex_get_from_cache(object_property(user, path));
    *secondlastcolon=':';
    *lastcolon=':';
    if(objectEditing){
      if(!*buf) object_property_set(objectEditing, secondlastcolon+1, (char*)"");
      else object_property_set(objectEditing, secondlastcolon+1, buf);
    }
  }
}

void set_property_name(char* path , char* name)
{
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  object_property_set(objectEditing, name, (char*)"--");
}

void set_property_name_and_object(char* path , char* name)
{
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  object* o = create_new_object_for_property_name(path, name);
  if(o) object_property_set(objectEditing, name, object_property(o, (char*)"UID"));
  else object_property_set(objectEditing, name, (char*)"--");
}

void set_property_name_and_link(char* path , char* name)
{
  char* lastlink=pop_last((char*)"viewing-r");
  if(!lastlink) return;
  object* objectEditing = onex_get_from_cache(object_property(user, path));
  object_property_set(objectEditing, name, lastlink);
}

char* pop_last(char* path)
{
  uint16_t len=object_property_length(user, path);
  char popPath[64]; snprintf(popPath, 64, "%s:%d", path, len);
  char* last = object_property(user, popPath);
  object_property_set(user, popPath, 0);
  return last;
}

// -------------

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

static bool FilterAutoInputText(const char* id, char* buf, int buflen, ImGuiTextEditCallback fafn)
{
  int flags=ImGuiInputTextFlags_CallbackCharFilter|ImGuiInputTextFlags_CallbackAlways|ImGuiInputTextFlags_EnterReturnsTrue;
  bool done=ImGui::InputText(id, buf, buflen, flags, fafn, buf);
  if(done){ ss= -1; se= -1; }
  return done;
}

// -------------

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


static int filter_and_autocomplete_calendar_tags(ImGuiTextEditCallbackData* data)
{
  int tls=object_property_size(config, (char*)"taglookup");
  char* calendarTags[tls-1];
  for(int t=2; t<=tls; t++){
    calendarTags[t-2]=object_property_key(config, (char*)"taglookup", t);
  }
  return filter_and_autocomplete(data, enforcePropertyName, calendarTags, tls-1);
}

static int filter_and_autocomplete_default(ImGuiTextEditCallbackData* data)
{
  return filter_and_autocomplete(data, 0, 0, 0);
}

void draw_new_property_value_editor(char* path, char* propname, char* val, bool single, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
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
      if(locallyEditable){
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
    if(height==buttonHeight) done=FilterAutoInputText(valId, valBuf, IM_ARRAYSIZE(valBuf), faa);
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

static int filter_and_autocomplete_property_names(ImGuiTextEditCallbackData* data)
{
  return filter_and_autocomplete(data, enforcePropertyName, (char**)propertyNameChoices, IM_ARRAYSIZE(propertyNameChoices));
}

void draw_object_footer(char* path, bool locallyEditable, int16_t width, int16_t keyWidth, int8_t depth)
{
  if(depth>=3) return;
  if(!locallyEditable) return;
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
    int c=0;
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
    if(FilterAutoInputText("## property name", propNameBuf, IM_ARRAYSIZE(propNameBuf), filter_and_autocomplete_property_names)){
      if(*propNameBuf){
        if(!strcmp(propNameBuf, "Rules")) set_property_name_and_object(path, propNameBuf);
        else if(!strcmp(propNameBuf, "Notifying")) set_property_name_and_link(path, propNameBuf);
        else set_property_name(path, propNameBuf);
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
  char pathj[256]; snprintf(pathj, 256, "%s:%d", path, j);
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
    object* objectEditing = onex_get_from_cache(object_property(user, path));
    *lastcolon=':';
    object* o = create_new_object_like_others(path);
    if(o){
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        object_property_set(objectEditing, lastcolon+1, object_property(o, (char*)"UID"));
      }
      else object_property_add(objectEditing, lastcolon+1, object_property(o, (char*)"UID"));
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
      object* objectEditing = onex_get_from_cache(object_property(user, path));
      *lastcolon=':';
      if(object_property_is(objectEditing, lastcolon+1, (char*)"--")){
        object_property_set(objectEditing, lastcolon+1, lastlink);
      }
      else object_property_add(objectEditing, lastcolon+1, lastlink);
    }
  }
  if(!linkFrom) track_drag(addLnkId, false);
  ImGui::SameLine();
  track_link(true, path, w+smallButtonWidth, buttonHeight);

  ImGui::PopStyleColor(4);
}

object* create_new_object_for_property_name(char* path, char* name)
{
  object* r=object_new(0, (char*)"default", 0, 4);
  char* is;
  if(!strcmp(name,"Rules")) is=(char*)"Rule";
  else is=name;
  object_property_set(r, (char*)"is", is);
  return r;
}

object* create_new_object_like_others(char* path)
{
  object* r=object_new(0, (char*)"default", 0, 4);
  bool filled=false;
  int16_t ln = object_property_length(user, path);
  for(int i=1; i<=ln; i++){
    size_t l=strlen(path);
    snprintf(path+l, 128-l, ":%d", i);
    int8_t sz=object_property_size(user, path);
    if(sz>0) for(int j=1; j<=sz; j++){
      char* key=object_property_key(user, path, j);
      char* is=0; if(key && !strcmp(key,"is")) is=object_property_val(user, path, j);
      object_property_set(r, key, is? is: (char*)"--");
      filled=true;
    }
    path[l] = 0;
  }
  if(!filled) object_property_set(r, (char*)"is", (char*)"editable");
  return r;
}

int date_compare(struct tm* d1, struct tm* d2)
{
  int yd=(d1->tm_year - d2->tm_year); if(yd) return yd;
  int md=(d1->tm_mon  - d2->tm_mon);  if(md) return md;
  int dd=(d1->tm_mday - d2->tm_mday); if(dd) return dd;
  return 0;
}

void get_summary(char* path, char* summary)
{
  *summary=0;
  if(get_summary_from(path, summary, "title")) return;
  if(get_summary_from(path, summary, "name")) return;
  if(get_summary_from(path, summary, "summary")) return;
  if(get_summary_from(path, summary, "description")) return;
  if(get_summary_from(path, summary, "text")) return;
  if(get_summary_from(path, summary, "content")) return;
  if(get_summary_from(path, summary, "is")) return;
}

bool get_summary_from(char* path, char* summary, const char* key)
{
  char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s:%s", path, key);
  char* vals=object_property_values(user, pathkey);
  if(!vals) return false;
  snprintf(summary, 128, "%s ", vals);
  return true;
}

int16_t calculate_key_width(char* path)
{
  int16_t w=0;
  int8_t sz = object_property_size(user, path);
  if(sz>0) for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, path, i);
    if(key){
      int16_t l=strlen(key);
      if(l > w) w=l;
    }
  }
  return w>7? w*28: 196;
}

int16_t calculate_scroller_height(char* path, int16_t height)
{
  int16_t heightforscrollers=height-2.5*buttonHeight;
  int8_t  numberofscrollers=0;
  int8_t sz = object_property_size(user, path);
  if(sz>0) for(int i=1; i<=sz; i++){
    char* key=object_property_key(user, path, i);
    char pathkey[128]; size_t l = snprintf(pathkey, 128, "%s:%s", path, key);
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

void draw_property_list(char* path, char* key, bool locallyEditable, int16_t width, int16_t height, int16_t keyWidth, int8_t depth)
{
  if(width < 200) return;
  draw_key(path, key, width, height, keyWidth, depth);
  draw_nested_object_properties_list(path, locallyEditable, width-keyWidth, height, depth);
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

void draw_nested_object_properties_list(char* path, bool locallyEditable, int16_t width, int16_t height, int8_t depth)
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
      draw_new_property_value_editor(path, 0, textlines, true, locallyEditable, width, height, depth);
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
          draw_new_property_value_editor(path, 0, val, false, locallyEditable, width-rhsPadding, buttonHeight, depth);
        }else{
          bool locallyEditable = object_is_local(val);
          draw_object_properties(path, locallyEditable, width-rhsPadding, height, depth+1);
        }
        draw_padding(path, width-rhsPadding, paddingHeight, depth);
        path[l] = 0;
      }
      if(locallyEditable) draw_new_value_or_object_button(path, width-rhsPadding, j, depth, false);
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

static bool sameDay(struct tm* d1, struct tm* d2)
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
      if(sameDay(&thisdate, &todaydate) && thisdate.tm_mday==1){
        ImGui::PushStyleColor(ImGuiCol_Text, actionColour);
        ImGui::PushStyleColor(ImGuiCol_Button, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, propertyBackgroundActive);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, propertyBackgroundActive);
      }
      else
      if(sameDay(&thisdate, &todaydate)){
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

      if(sameDay(&thisdate, &todaydate) || thisdate.tm_mday==1) ImGui::PopStyleColor(4);
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

void ensureAlarm(object* o, time_t t)
{
  setAlarm(t, object_property(o, (char*)"UID"));
  object_keep_active(o, true);
}

void cancelAlarm(object* o)
{
  setAlarm(0, object_property(o, (char*)"UID"));
  object_keep_active(o, false);
}

bool evaluate_event(object* o, void* d)
{
  log_write("evaluate_event\n"); object_log(o);
  if(!object_property_contains(o, (char*)"is", (char*)"event")){   log_write("object is no longer an is: event\n");
    cancelAlarm(o);
    object_set_evaluator(o, (char*)"default");
    return true;
  }
  struct tm date; time_t t=get_date_from_object(o, (char*)"date", &date);
  todayseconds=time(0); todaydate = *localtime(&todayseconds);
  if(t== -1) date=todaydate;
  if(date_compare(&date, &todaydate) < 0){    log_write("event for past date, ignored\n");
    cancelAlarm(o);
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
    cancelAlarm(o);
    showNotification(title, text);
  }
  else{
    ensureAlarm(o, t);
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
            object* objectEditing = onex_get_from_cache(caluid);
            object_property_add(objectEditing, (char*)"list", evtuid);
          }
          else{
            object_property_add(user, path, evtuid);
            if(static_gui) static_gui->changed();
          }
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

