
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <boards.h>
#include <onex-kernel/boot.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onex-kernel/gpio.h>
#include <onex-kernel/i2c.h>
#include <onex-kernel/blenus.h>
#include <onex-kernel/gfx.h>
#include <onex-kernel/touch.h>
#include <onex-kernel/motion.h>
#include <onf.h>
#include <onr.h>
#include <lvgl.h>
#include <noto_sans_numeric_60.h>
#include <noto_sans_numeric_80.h>

static object* user;
static object* battery;
static object* bluetooth;
static object* touch;
static object* motion;
static object* button;
static object* backlight;
static object* oclock;
static object* watchface;
static object* home;
static object* about;

static char* deviceuid;
static char* useruid;
static char* batteryuid;
static char* bluetoothuid;
static char* touchuid;
static char* motionuid;
static char* buttonuid;
static char* backlightuid;
static char* clockuid;
static char* watchfaceuid;
static char* homeuid;
static char* aboutuid;

static volatile bool          event_dfu=false;
static volatile bool          event_tick_10ms=false;
static volatile touch_info_t  touch_info;
static volatile uint16_t      touch_info_stroke=0;
static volatile motion_info_t motion_info;
static volatile blenus_info_t ble_info={ .connected=false, .rssi=-100 };

static void every_10ms(){
  event_tick_10ms=true;
}

static void every_second(){
  if(gpio_get(BUTTON_1)!=BUTTONS_ACTIVE_STATE) boot_feed_watchdog();
  onex_run_evaluators(clockuid, 0);
}

static void every_10s(){
  onex_run_evaluators(batteryuid, 0);
}

static void blechanged(blenus_info_t bi)
{
  ble_info=bi;
  onex_run_evaluators(bluetoothuid, 0);
}

static bool backlight_on=false;

static void touched(touch_info_t ti)
{
  touch_info=ti;

  static uint16_t swipe_start_x=0;
  static uint16_t swipe_start_y=0;

  bool is_gesture=(touch_info.gesture==TOUCH_GESTURE_LEFT  ||
                   touch_info.gesture==TOUCH_GESTURE_RIGHT ||
                   touch_info.gesture==TOUCH_GESTURE_DOWN  ||
                   touch_info.gesture==TOUCH_GESTURE_UP      );

  if(touch_info.action==TOUCH_ACTION_CONTACT && !is_gesture){
    swipe_start_x=touch_info.x;
    swipe_start_y=touch_info.y;
    touch_info_stroke=0;
  }
  else
  if(touch_info.action!=TOUCH_ACTION_CONTACT && is_gesture){
    int16_t dx=touch_info.x-swipe_start_x;
    int16_t dy=touch_info.y-swipe_start_y;
    touch_info_stroke=(uint16_t)sqrtf(dx*dx+dy*dy);
  }

  onex_run_evaluators(touchuid, 0);


  static uint8_t  disable_user_touch=0;

  if(!backlight_on         && touch_info.action==TOUCH_ACTION_CONTACT) disable_user_touch=1;
  if(disable_user_touch==1 && touch_info.action!=TOUCH_ACTION_CONTACT) disable_user_touch=2;
  if(disable_user_touch==2 && touch_info.action==TOUCH_ACTION_CONTACT) disable_user_touch=0;

  if(!disable_user_touch){
    onex_run_evaluators(useruid, (void*)1);
  }
}

static void moved(motion_info_t mi)
{
  motion_info=mi;
  onex_run_evaluators(motionuid, 0);
}

static void button_changed(uint8_t pin, uint8_t type){
  onex_run_evaluators(buttonuid, 0);
}

static void charging_changed(uint8_t pin, uint8_t type){
  onex_run_evaluators(batteryuid, 0);
}

static bool evaluate_user(object* o, void* d);
static bool evaluate_battery_io(object* o, void* d);
static bool evaluate_bluetooth_io(object* o, void* d);
static bool evaluate_touch_io(object* o, void* d);
static bool evaluate_motion_io(object* o, void* d);
static bool evaluate_button_io(object* o, void* d);
static bool evaluate_backlight_io(object* o, void* d);

static void draw_ui();
//static void draw_obj();

#define ADC_CHANNEL 0

#if defined(LOG_TO_GFX)
extern volatile char* event_log_buffer;

void draw_log()
{
  char* s=(char*)event_log_buffer;
  char* nl=strchr(s, '\n');
  char* s2=0;
  if(nl){
    *nl=0;
    s2=nl+1;
  }
  if(strlen(s)>16) s[16]=0;
  if(nl && strlen(s2)>16) s2[16]=0;
  gfx_push(10,200);
  gfx_text(s);
  if(nl){
    gfx_push(10,220);
    gfx_text(s2);
    gfx_pop();
  }
  gfx_pop();
}
#endif

static void clear_screen()
{
  gfx_screen_colour(GFX_BLACK);
  gfx_screen_fill();
}

static void init_lv();

int main()
{
  boot_init();
  log_init();
  time_init();
  gpio_init();
  blenus_init(0, blechanged);

  init_lv();

  gfx_init();
  gfx_text_colour(GFX_BLUE);
  clear_screen();

  touch_init(touched);
  motion_init(moved);

  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, RISING_AND_FALLING, button_changed);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
  gpio_mode_cb(CHARGE_SENSE, INPUT, RISING_AND_FALLING, charging_changed);
  gpio_adc_init(BATTERY_V, ADC_CHANNEL);

  gpio_mode(LCD_BACKLIGHT_LOW, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_MID, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);

  onex_init("");

  onex_set_evaluators("device",    evaluate_device_logic, 0);
  onex_set_evaluators("user",      evaluate_user, 0);
  onex_set_evaluators("battery",   evaluate_battery_io, 0);
  onex_set_evaluators("bluetooth", evaluate_bluetooth_io, 0);
  onex_set_evaluators("touch",     evaluate_touch_io, 0);
  onex_set_evaluators("motion",    evaluate_motion_io, 0);
  onex_set_evaluators("button",    evaluate_button_io, 0);
  onex_set_evaluators("backlight", evaluate_edit_rule, evaluate_light_logic, evaluate_backlight_io, 0);
  onex_set_evaluators("clock",     evaluate_clock_sync, evaluate_clock, 0);
  onex_set_evaluators("editable",  evaluate_edit_rule, 0);

  object_set_evaluator(onex_device_object, "device");

  user     =object_new(0, "user",      "user", 8);
  battery  =object_new(0, "battery",   "battery", 4);
  bluetooth=object_new(0, "bluetooth", "bluetooth", 4);
  touch    =object_new(0, "touch",     "touch", 6);
  motion   =object_new(0, "motion",    "motion", 8);
  button   =object_new(0, "button",    "button", 4);
  backlight=object_new(0, "backlight", "editable light", 9);
  oclock   =object_new(0, "clock",     "clock event", 12);
  watchface=object_new(0, "editable",  "editable watchface", 6);
  home     =object_new(0, "editable",  "editable", 4);
  about    =object_new(0, "editable",  "editable", 4);

  deviceuid   =object_property(onex_device_object, "UID");
  useruid     =object_property(user, "UID");
  batteryuid  =object_property(battery, "UID");
  bluetoothuid=object_property(bluetooth, "UID");
  touchuid    =object_property(touch, "UID");
  motionuid   =object_property(motion, "UID");
  buttonuid   =object_property(button, "UID");
  backlightuid=object_property(backlight, "UID");
  clockuid    =object_property(oclock, "UID");
  watchfaceuid=object_property(watchface, "UID");
  homeuid     =object_property(home, "UID");
  aboutuid    =object_property(about, "UID");

  object_property_set(backlight, "light", "on");
  object_property_set(backlight, "level", "high");
  object_property_set(backlight, "timeout", "5000");
  object_property_set(backlight, "touch", touchuid);
  object_property_set(backlight, "motion", motionuid);
  object_property_set(backlight, "button", buttonuid);

  object_property_set(oclock, "title", "OnexOS Clock");
  object_property_set(oclock, "ts", "%unknown");
  object_property_set(oclock, "tz", "%unknown");
  object_property_set(oclock, "device", deviceuid);

  object_property_set(watchface, "clock", clockuid);
  object_property_set(watchface, "ampm-24hr", "ampm");

  object_property_set(home, (char*)"battery", batteryuid);
  object_property_set(home, (char*)"bluetooth", bluetoothuid);
  object_property_set(home, (char*)"watchface", watchfaceuid);

  object_property_set(user, "viewing", homeuid);

  object_property_add(onex_device_object, (char*)"user", useruid);
  object_property_add(onex_device_object, (char*)"io",   batteryuid);
  object_property_add(onex_device_object, (char*)"io",   bluetoothuid);
  object_property_add(onex_device_object, (char*)"io",   touchuid);
  object_property_add(onex_device_object, (char*)"io",   motionuid);
  object_property_add(onex_device_object, (char*)"io",   buttonuid);
  object_property_add(onex_device_object, (char*)"io",   backlightuid);
  object_property_add(onex_device_object, (char*)"io",   clockuid);

  onex_run_evaluators(useruid, 0);
  onex_run_evaluators(batteryuid, 0);
  onex_run_evaluators(bluetoothuid, 0);
  onex_run_evaluators(clockuid, 0);
  onex_run_evaluators(backlightuid, 0);

  time_ticker(every_10ms,      10);
  time_ticker(every_second,  1000);
  time_ticker(every_10s,    10000);

  while(1){

    if(!onex_loop()){
      gfx_spi_sleep();
      i2c_sleep(); // will i2c_wake() in irq to read values
      gpio_sleep();

      boot_sleep();

      gpio_wake();
      gfx_spi_wake();
    }

    if(event_dfu){
      boot_dfu_start();
    }
    if(event_tick_10ms){
      event_tick_10ms=false;
      lv_task_handler();
    }
#if defined(LOG_TO_GFX)
    if(event_log_buffer){
      draw_log();
      event_log_buffer=0;
    }
#endif
  }
}

#define BATTERY_ZERO_PERCENT 3400
#define BATTERY_100_PERCENT 4100
#define BATTERY_PERCENT_STEPS 5
bool evaluate_battery_io(object* o, void* d)
{
  char b[16];

  int32_t bv = gpio_read(ADC_CHANNEL);
  int32_t mv = bv*2000/(1024/(33/10));
  int8_t pc = ((mv-BATTERY_ZERO_PERCENT)*100/((BATTERY_100_PERCENT-BATTERY_ZERO_PERCENT)*BATTERY_PERCENT_STEPS))*BATTERY_PERCENT_STEPS;
  if(pc<0) pc=0;
  if(pc>100) pc=100;
  snprintf(b, 16, "%d%%(%ld)", pc, mv);

  object_property_set(battery, "percent", b);

  int batt=gpio_get(CHARGE_SENSE);
  snprintf(b, 16, "%s", batt? "powering": "charging");
  object_property_set(battery, "status", b);

  return true;
}

bool evaluate_bluetooth_io(object* o, void* d)
{
  object_property_set(bluetooth, "connected", ble_info.connected? "yes": "no");
  char buf[16];
  snprintf(buf, 16, "%3d", ble_info.rssi);
  object_property_set(bluetooth, "rssi", buf);
  return true;
}

bool evaluate_touch_io(object* o, void* d)
{
  char buf[64];

  snprintf(buf, 64, "%3d %3d", touch_info.x, touch_info.y);
  object_property_set(touch, "coords", buf);

  snprintf(buf, 64, "%s %s", touch_actions[touch_info.action], touch_gestures[touch_info.gesture]);
  object_property_set(touch, "action", buf);

  snprintf(buf, 64, "%d", touch_info_stroke);
  object_property_set(touch, "stroke", buf);

  return true;
}

bool evaluate_motion_io(object* o, void* d)
{
  static int16_t prevx=0;
  static int16_t prevm=0;
  bool viewscreen=(prevx < -300 &&
                   motion_info.x < -700 &&
                   motion_info.x > -1200 &&
                   abs(motion_info.y) < 600 &&
                   abs(prevm) > 70);
  prevx=motion_info.x;
  prevm=motion_info.m;

  static uint32_t ticks=0;
  ticks++;
  if(ticks%50 && !viewscreen) return true;

  char buf[64];
  snprintf(buf, 64, "%d %d %d %d", motion_info.x, motion_info.y, motion_info.z, motion_info.m);
  object_property_set(motion, "x-y-z-m", buf);
  object_property_set(motion, "gesture", viewscreen? "view-screen": "none");

  return true;
}

bool evaluate_button_io(object* o, void* d)
{
  bool button_pressed=(gpio_get(BUTTON_1)==BUTTONS_ACTIVE_STATE);
  object_property_set(button, "state", button_pressed? "down": "up");
  return true;
}

bool evaluate_backlight_io(object* o, void* d)
{
  if(object_property_is(backlight, "light", "on")){
    backlight_on=true;
    bool mid =object_property_is(backlight, "level", "mid");
    bool high=object_property_is(backlight, "level", "high");
  //touch_wake();
    gfx_wake();
    gpio_set(LCD_BACKLIGHT_LOW,               LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  (mid||high)? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_HIGH, (high)?      LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
  } else {
    backlight_on=false;
    gpio_set(LCD_BACKLIGHT_LOW,  !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_HIGH, !LEDS_ACTIVE_STATE);
    gfx_sleep();
  //touch_sleep();
  }
  return true;
}

static lv_obj_t* home_screen;
static lv_obj_t* about_screen;

bool evaluate_user(object* o, void* touchevent)
{
  if(touchevent){
    if(touch_info.gesture==TOUCH_GESTURE_LEFT  && touch_info_stroke > 50 && object_property_is(user, "viewing", homeuid )){
      lv_scr_load(about_screen); // switching screens should be by viewing:is detection, and knowing what you're on
      object_property_set(user, "viewing", aboutuid);
    }
    else
    if(touch_info.gesture==TOUCH_GESTURE_RIGHT && touch_info_stroke > 50 && object_property_is(user, "viewing", aboutuid)){
      lv_scr_load(home_screen);
      object_property_set(user, "viewing", homeuid);
    }
    else
    if(touch_info.gesture==TOUCH_GESTURE_TAP_LONG && object_property_is(user, "viewing", aboutuid)){
      event_dfu=true;
    }
  }
  draw_ui();
  return true;
}

#define PADDING 2
#define L_PADDING 5
#define T_PADDING 5
#define ROW_HEIGHT 26
#define PROPERTY_WIDTH 100
#define VALUE_WIDTH 130
#define SCREEN_WIDTH 235
#define SCREEN_HEIGHT 235

#define ACTION_BG       GFX_RGB256(245,245,100)
#define ACTION_COLOUR   GFX_RGB256(128,26,51)
#define PROPERTY_BG     GFX_RGB256(247,255,250)
#define PROPERTY_COLOUR GFX_RGB256(51,128,77)
#define VALUE_BG        GFX_RGB256(245,222,255)
#define VALUE_COLOUR    GFX_RGB256(51,51,100)
#define BATTERY_LOW     GFX_RGB256(200,0,0)
#define BATTERY_MED     GFX_RGB256(200,200,0)
#define BATTERY_HIGH    GFX_RGB256(0,200,0)
#define BATTERY_CHG     GFX_RGB256(200,200,200)
#define BLE_CONNECTED   GFX_RGB256(0,0,255)

void draw_area_and_ready(lv_disp_drv_t * disp, const lv_area_t* area, lv_color_t* color_p)
{
  gfx_draw_area(area->x1, area->x2, area->y1, area->y2, (uint16_t*)color_p);
  lv_disp_flush_ready(disp);
}

static lv_disp_buf_t disp_buf;

#define LV_BUF_SIZE (LV_HOR_RES_MAX * 6)
static lv_color_t lv_buf1[LV_BUF_SIZE];
static lv_color_t lv_buf2[LV_BUF_SIZE];

static lv_obj_t* time_label;
static lv_obj_t* date_label;
static lv_obj_t* boot_label;

void init_lv()
{
  lv_init();
  lv_disp_buf_init(&disp_buf, lv_buf1, lv_buf2, LV_BUF_SIZE);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = draw_area_and_ready;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  home_screen  = lv_obj_create(0,0);
  about_screen = lv_obj_create(0,0);

  time_label=lv_label_create(home_screen, 0);
  lv_label_set_long_mode(time_label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(time_label, 240);
  lv_obj_set_height(time_label, 200);
  lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(time_label, home_screen, LV_ALIGN_CENTER, -5, -20);

  date_label=lv_label_create(home_screen, 0);
  lv_label_set_long_mode(date_label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(date_label, 200);
  lv_obj_set_height(date_label, 200);
  lv_label_set_align(date_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(date_label, home_screen, LV_ALIGN_CENTER, -5, 50);

  boot_label=lv_label_create(about_screen, 0);
  lv_label_set_long_mode(boot_label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(boot_label, 200);
  lv_obj_set_height(boot_label, 200);
  lv_label_set_align(boot_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(boot_label, about_screen, LV_ALIGN_CENTER, -5, -50);

  lv_style_t bg;
  lv_style_copy(&bg, &lv_style_plain);
  bg.body.main_color = LV_COLOR_BLACK;
  bg.body.grad_color = LV_COLOR_BLACK;
  bg.text.color      = LV_COLOR_WHITE;
  bg.image.color     = LV_COLOR_WHITE;
  lv_label_set_style(home_screen, LV_LABEL_STYLE_MAIN, &bg);
  lv_label_set_style(about_screen, LV_LABEL_STYLE_MAIN, &bg);

  lv_style_t lb;
  lv_style_copy(&lb, &bg);
  lb.text.font= &noto_sans_numeric_80;
  lv_label_set_style(time_label, LV_LABEL_STYLE_MAIN, &lb);

  lv_label_set_text(time_label, "00:00");
  lv_label_set_text(date_label, "Onex");

  lv_scr_load(home_screen);
}

extern char __BUILD_TIMESTAMP;
extern char __BOOTLOADER_NUMBER;

static void draw_home();
static void draw_about();

void draw_ui()
{
  if(object_property_is(user, "viewing", homeuid))  draw_home();
  if(object_property_is(user, "viewing", aboutuid)) draw_about();
}

void draw_home()
{
  char* pc=object_property(   user, "viewing:battery:percent");
  bool  ch=object_property_is(user, "viewing:battery:status", "charging");
  bool  bl=object_property_is(user, "viewing:bluetooth:connected", "yes");
  char* ts=object_property(   user, "viewing:watchface:clock:ts");
  char* tz=object_property(   user, "viewing:watchface:clock:tz:2");
  bool h24=object_property_is(user, "viewing:watchface:ampm-24hr", "24hr");

  if(!ts) return;

  char* e;

  uint64_t tsnum=strtoull(ts,&e,10);
  if(*e) return;

  uint32_t tznum=strtoul(tz?tz:"0",&e,10);
  if(*e) tznum=0;

  time_t est=(time_t)(tsnum+tznum);
  struct tm tms={0};
  localtime_r(&est, &tms);

  char t[32];

  strftime(t, 32, h24? "%H:%M": "%l:%M", &tms);
  lv_label_set_text(time_label, t);

  strftime(t, 32, h24? "24 %a %d %h": "%p %a %d %h", &tms);
  lv_label_set_text(date_label, t);

  int8_t pcnum=(int8_t)strtol(pc,&e,10);
  if(pcnum<5) pcnum=5;
  if(pcnum>100) pcnum=100;

  uint16_t batt_col;
  if(ch)       batt_col=BATTERY_CHG;
  else
  if(pcnum>67) batt_col=BATTERY_HIGH;
  else
  if(pcnum>33) batt_col=BATTERY_MED;
  else         batt_col=BATTERY_LOW;

  int8_t blnum=(int8_t)50;
  uint16_t ble_col=bl? BLE_CONNECTED: GFX_GREY_7;

  #define INDICATOR_PAD    2
  #define INDICATOR_HEIGHT 2
  #define INDICATOR_WIDTH  (SCREEN_WIDTH-2*INDICATOR_PAD)

  gfx_rect_fill(INDICATOR_PAD,0,                 INDICATOR_WIDTH,            INDICATOR_HEIGHT, GFX_GREY_3);
  gfx_rect_fill(INDICATOR_PAD,0,                (INDICATOR_WIDTH*pcnum)/100, INDICATOR_HEIGHT, batt_col);

  gfx_rect_fill(INDICATOR_PAD,INDICATOR_HEIGHT+INDICATOR_PAD,  INDICATOR_WIDTH,            INDICATOR_HEIGHT, GFX_GREY_3);
  gfx_rect_fill(INDICATOR_PAD,INDICATOR_HEIGHT+INDICATOR_PAD, (INDICATOR_WIDTH*blnum)/100, INDICATOR_HEIGHT, ble_col);

  gfx_text_colour(GFX_BLUE);
  gfx_push(10,220);
  gfx_text((time_es()%2)? "/": "\\");
  gfx_pop();
}

void draw_about()
{
  gfx_text_colour(GFX_WHITE);
  lv_label_set_text(boot_label, "OnexOS update");
  char b[32]; snprintf(b, 32, ((time_es()%2)? "%lu %lu %d/": "%lu %lu %d\\"), (unsigned long)&__BUILD_TIMESTAMP, (unsigned long)&__BOOTLOADER_NUMBER, (uint8_t)((DWT->CYCCNT)%256));
  gfx_pos(10,220);
  gfx_text(b);
}

/*
void draw_obj()
{
  gfx_rect_line(0,0, SCREEN_WIDTH,SCREEN_HEIGHT, GFX_GREY_F, PADDING);

  gfx_screen_colour(ACTION_BG);
  gfx_rect_fill(PADDING,PADDING, SCREEN_WIDTH-PADDING,ROW_HEIGHT, ACTION_BG);
  gfx_pos(PADDING+L_PADDING, PADDING+T_PADDING);
  gfx_text_colour(ACTION_COLOUR);
  gfx_text("user");

  gfx_rect_fill(PADDING,PADDING+ROW_HEIGHT, PADDING+PROPERTY_WIDTH,PADDING+ROW_HEIGHT*2, PROPERTY_BG);
  gfx_pos(PADDING+L_PADDING, PADDING+T_PADDING);
  gfx_text_colour(PROPERTY_COLOUR);
  gfx_text("is");

  gfx_rect_fill(PADDING,PADDING+ROW_HEIGHT, PADDING+PROPERTY_WIDTH,PADDING+ROW_HEIGHT*2, VALUE_BG);
  gfx_pos(PADDING+L_PADDING, PADDING+T_PADDING);
  gfx_text_colour(VALUE_COLOUR);
  gfx_text("list");
}
*/

