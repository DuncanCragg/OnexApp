
#include <string.h>
#include <boards.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onex-kernel/gpio.h>
#include <onex-kernel/blenus.h>
#include <onex-kernel/gfx.h>
#include <onex-kernel/touch.h>
#include <onf.h>
#include <onr.h>
#include <lvgl.h>
#include <noto_sans_numeric_60.h>
#include <noto_sans_numeric_80.h>

object* sensors;
object* controllers;
object* oclock;
object* user;

char* deviceuid;
char* sensorsuid;
char* controllersuid;
char* clockuid;
char* useruid;

static volatile bool event_tick_10ms=false;
static volatile bool event_tick_sec=false;
static volatile bool event_tick_min=false;
static volatile bool event_button=false;
static volatile bool event_touch=false;

static volatile bool         pressed;
static volatile touch_info_t touchinfo;

static void every_10ms(){             event_tick_10ms=true; }
static void every_second(){           event_tick_sec=true; }
static void every_minute(){           event_tick_min=true; }
static void button_changed(int p){    event_button=true; pressed=p; }
static void touched(touch_info_t ti){ event_touch=true;  touchinfo=ti; }

static bool evaluate_user(object* o, void* d);
static bool evaluate_sensors_io(object* o, void* d);
static bool evaluate_controllers_io(object* o, void* d);

static void draw_ui();

#define ADC_CHANNEL 0

void* x;
#define WHERESTHEHEAP(s) x = malloc(1); log_write("heap after %s: %x\n", s, x);

extern volatile char* event_log_buffer;

void draw_log(char* s)
{
  char* nl=strchr(s, '\n');
  char* s2;
  if(nl){
    *nl=0;
    s2=nl+1;
  }
  if(strlen(s)>9) s[9]=0;
  if(nl && strlen(s2)>9) s2[9]=0;
  gfx_push(10,150);
  gfx_text(s);
  if(nl){
    gfx_push(10,190);
    gfx_text(s2);
    gfx_pop();
  }
  gfx_pop();
}

static void init_ui();

int main()
{
  log_init();
  time_init();
  gpio_init();
  blenus_init(0);

  init_ui();

  gfx_init();
  gfx_screen_colour(GFX_BLACK);
  gfx_text_colour(GFX_BLUE);
  gfx_screen_fill();

  touch_init(touched);

  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, button_changed);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
  gpio_mode(CHARGE_SENSE, INPUT);
  gpio_adc_init(BATTERY_V, ADC_CHANNEL);

  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);

  onex_init("");

  onex_set_evaluators("device",      evaluate_device_logic, 0);
  onex_set_evaluators("user",        evaluate_user, 0);
  onex_set_evaluators("sensors",     evaluate_sensors_io, 0);
  onex_set_evaluators("controllers", evaluate_edit_rule, evaluate_controllers_io, 0);
  onex_set_evaluators("clock",       evaluate_clock_sync, evaluate_clock, 0);

  object_set_evaluator(onex_device_object, "device");

  user       =object_new(0, "user",        "user", 8);
  sensors    =object_new(0, "sensors",     "sensors", 8);
  controllers=object_new(0, "controllers", "editable controllers", 4);
  oclock     =object_new(0, "clock",       "clock event", 12);

  deviceuid     =object_property(onex_device_object, "UID");
  useruid       =object_property(user, "UID");
  sensorsuid    =object_property(sensors, "UID");
  controllersuid=object_property(controllers, "UID");
  clockuid      =object_property(oclock, "UID");

  object_property_set(oclock, "title", "Clock");
  object_property_set(oclock, "timezone", "GMT");
  object_property_add(oclock, "timezone", "BST");
  object_property_set(oclock, "device", deviceuid);

  object_property_add(onex_device_object, (char*)"user", useruid);
  object_property_add(onex_device_object, (char*)"io",   sensorsuid);
  object_property_add(onex_device_object, (char*)"io",   controllersuid);
  object_property_add(onex_device_object, (char*)"io",   clockuid);

  object_property_set(user, "viewing", clockuid);
  object_property_set(controllers, "backlight", "on");

  onex_run_evaluators(useruid, 0);
  onex_run_evaluators(sensorsuid, false);
  onex_run_evaluators(controllersuid, 0);

  time_ticker(every_10ms,     10);
  time_ticker(every_second, 1000);
  time_ticker(every_minute, 60000);

  draw_ui();

  while(1){

    onex_loop();

    if(event_tick_10ms){
      event_tick_10ms=false;
      lv_task_handler();
    }
    if(event_tick_sec){
      event_tick_sec=false;
      onex_run_evaluators(clockuid, 0);
    }
    if(event_tick_min){
      event_tick_min=false;
      onex_run_evaluators(sensorsuid, 0);
    }
    if(event_button){
      event_button=false;
      onex_run_evaluators(sensorsuid, pressed? "down": "up");
    }
    if(event_touch){
      event_touch=false;
      onex_run_evaluators(controllersuid, (touchinfo.gesture==TOUCH_GESTURE_TAP_LONG)? "off": "on");
    }
    if(event_log_buffer){
      draw_log((char*)event_log_buffer);
      event_log_buffer=0;
    }
  }
}

bool evaluate_sensors_io(object* o, void* pressed)
{
  char b[16];

  int16_t bv = gpio_read(ADC_CHANNEL);
  int16_t mv = bv*2000/(1024/(33/10));
  int8_t  pc = ((mv-3520)*100/5200)*10;
  snprintf(b, 16, "%d%%(%d)", pc, mv);

  object_property_set(sensors, "battery-percent", b);

  int batt=gpio_get(CHARGE_SENSE);
  snprintf(b, 16, "%s", batt? "battery": "charging");
  object_property_set(sensors, "battery-charge", b);

  if(pressed) object_property_set(sensors, "button", pressed);

  return true;
}

bool evaluate_controllers_io(object* o, void* backlight)
{
  if(backlight) object_property_set(controllers, "backlight", (char*)backlight);

  if(object_property_is(controllers, "backlight", "on")){
    gpio_set(LCD_BACKLIGHT_HIGH, LEDS_ACTIVE_STATE);
  } else {
    gpio_set(LCD_BACKLIGHT_HIGH, !LEDS_ACTIVE_STATE);
  }
  return true;
}

bool evaluate_user(object* o, void* d)
{
  draw_ui();
  return true;
}

#define PADDING 2
#define L_PADDING 5
#define T_PADDING 5
#define ROW_HEIGHT 26
#define PROPERTY_WIDTH 100
#define VALUE_WIDTH 130
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define ACTION_BG       GFX_RGB256(245,245,100)
#define ACTION_COLOUR   GFX_RGB256(128,26,51)
#define PROPERTY_BG     GFX_RGB256(247,255,250)
#define PROPERTY_COLOUR GFX_RGB256(51,128,77)
#define VALUE_BG        GFX_RGB256(245,222,255)
#define VALUE_COLOUR    GFX_RGB256(51,51,100)

void draw_area_and_ready(lv_disp_drv_t * disp, const lv_area_t* area, lv_color_t* color_p)
{
  gfx_draw_area(area->x1, area->x2, area->y1, area->y2, (uint16_t*)color_p);
  lv_disp_flush_ready(disp);
}

static lv_disp_buf_t disp_buf;
static lv_color_t lv_buffer[LV_HOR_RES_MAX * 10];
static lv_obj_t* big_time;

void init_ui()
{
  lv_init();
  lv_disp_buf_init(&disp_buf, lv_buffer, NULL, LV_HOR_RES_MAX * 10);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = draw_area_and_ready;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  big_time=lv_label_create(lv_scr_act(), 0);
  lv_label_set_long_mode(big_time, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(big_time, 240);
  lv_obj_set_height(big_time, 200);
  lv_label_set_align(big_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(big_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -30);

  lv_style_t st;
  lv_style_copy(&st, &lv_style_plain);
  st.body.main_color = LV_COLOR_BLACK;
  st.body.grad_color = LV_COLOR_BLACK;
  st.image.color     = LV_COLOR_WHITE;
  st.text.color      = LV_COLOR_BLACK;
  st.text.font = &noto_sans_numeric_60;
  lv_label_set_style(big_time, LV_LABEL_STYLE_MAIN, &st);
}

void draw_ui()
{
  char* t=object_property(user, "viewing:time");
  if(t) lv_label_set_text(big_time, t);
/*
  gfx_rect_line(0,0, SCREEN_WIDTH,SCREEN_HEIGHT, GFX_GREY_F, PADDING);

  gfx_screen_colour(ACTION_BG);
  gfx_rect_fill(PADDING,PADDING, SCREEN_WIDTH-PADDING,ROW_HEIGHT, ACTION_BG);
  gfx_pos(PADDING+L_PADDING, PADDING+T_PADDING);
  gfx_text_colour(ACTION_COLOUR);
  gfx_text("user");
*/
}

