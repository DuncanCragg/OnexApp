
#include "gui.h"

extern GUI* static_gui;
extern VulkanBase* app;
extern object* user;
extern object* config;

extern unsigned char* fontData;
extern int texWidth, texHeight;

extern bool keyboardCancelled;

void init_imgui(float width, float height);
void get_font_info();
void set_scaling();

void set_time_save_days();

void draw_gui();

