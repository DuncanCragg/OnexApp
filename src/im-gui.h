
#include "gui.h"

extern GUI* static_gui;
extern VulkanBase* app;
extern object* user;
extern object* config;

extern unsigned char* fontData;
extern int texWidth, texHeight;

extern bool keyboardCancelled;

extern uint16_t buttonHeight;
extern uint16_t paddingHeight;
extern uint16_t objectHeight;
extern uint16_t listHeight;

extern uint16_t shorterValWidth;
extern uint16_t buttonWidth;
extern uint16_t smallButtonWidth;
extern uint16_t rhsPadding;

extern uint16_t workspace1Width;
extern uint16_t workspace1Height;
extern uint16_t workspace2Width;
extern uint16_t workspace2Height;

extern bool calendarView;

extern ImVec4 actionColour;
extern ImVec4 actionBackground;
extern ImVec4 actionBackgroundActive;

extern ImVec4 propertyColour;
extern ImVec4 propertyBackground;
extern ImVec4 propertyBackgroundActive;

extern ImVec4 valueBackground;
extern ImVec4 valueBackgroundActive;

extern ImVec4 listBackground;
extern ImVec4 listBackgroundDark;

extern ImVec4 renderColour;
extern ImVec4 renderColourSoft;
extern ImVec4 renderBackground;
extern ImVec4 renderBackgroundActive;

extern ImVec4 schemeBrown;
extern ImVec4 schemeYellow;
extern ImVec4 schemeMauve;
extern ImVec4 schemePurple;
extern ImVec4 schemeGreen;
extern ImVec4 schemeLightPurple;
extern ImVec4 schemeDarkerPurple;
extern ImVec4 schemePlum;

extern char* dragPathId;

void init_imgui(float width, float height);
void get_font_info();
void set_scaling();

void draw_gui();

void toggle_open(char* path);
bool is_open(char* path);
void set_drag_scroll(char* path);
void track_drag(char* pathId, bool twodimensions);
void kill_drag();
void show_keyboard(float multy);
void hide_keyboard();

