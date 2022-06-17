
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
#include <onex-kernel/spi.h>
#include <onex-kernel/blenus.h>
#include <onex-kernel/display.h>
#include <onex-kernel/touch.h>
#include <onex-kernel/motion.h>
#include <onf.h>
#include <onr.h>
#include <lvgl.h>
#include <lv_font.h>
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
static object* viewlist;
static object* home;
static object* calendar;
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
static char* viewlistuid;
static char* homeuid;
static char* calendaruid;
static char* aboutuid;

static volatile bool          event_tick_5ms=false;
static volatile touch_info_t  touch_info;
static volatile uint16_t      touch_info_stroke=0;
static volatile motion_info_t motion_info;
static volatile blenus_info_t ble_info={ .connected=false, .rssi=-100 };

static char buf[64];

static void every_5ms(){
  event_tick_5ms=true;
}

static void every_second(){
  onex_run_evaluators(clockuid, 0);
  onex_run_evaluators(aboutuid, 0);
}

static void every_10s(){
  onex_run_evaluators(batteryuid, 0);
}

static void blechanged(blenus_info_t bi)
{
  ble_info=bi;
  onex_run_evaluators(bluetoothuid, 0);
}

static bool user_active=true;

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

  if(!user_active          && touch_info.action==TOUCH_ACTION_CONTACT) disable_user_touch=1;
  if(disable_user_touch==1 && touch_info.action!=TOUCH_ACTION_CONTACT) disable_user_touch=2;
  if(disable_user_touch==2 && touch_info.action==TOUCH_ACTION_CONTACT) disable_user_touch=0;

  if(!disable_user_touch){
    log_write("eval user from touched %d\n", is_gesture);
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

static bool evaluate_default(object* o, void* d);
static bool evaluate_user(object* o, void* d);
static bool evaluate_battery_in(object* o, void* d);
static bool evaluate_bluetooth_in(object* o, void* d);
static bool evaluate_touch_in(object* o, void* d);
static bool evaluate_motion_in(object* o, void* d);
static bool evaluate_button_in(object* o, void* d);
static bool evaluate_about_in(object* o, void* d);
static bool evaluate_backlight_out(object* o, void* d);

static void init_lv();

#if defined(LOG_TO_GFX)
extern volatile char* event_log_buffer;
static void draw_log();
#endif

#define ADC_CHANNEL 0

int main()
{
  boot_init();
  log_init();
  time_init();
  gpio_init();
  blenus_init(0, blechanged);

  display_init();

  touch_init(touched);
  motion_init(moved);

  init_lv();

  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, RISING_AND_FALLING, button_changed);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
  gpio_mode_cb(CHARGE_SENSE, INPUT, RISING_AND_FALLING, charging_changed);
  gpio_adc_init(BATTERY_V, ADC_CHANNEL);

  gpio_mode(LCD_BACKLIGHT_LOW, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_MID, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);

  onex_init("");

  onex_set_evaluators("default",   evaluate_default, 0);
  onex_set_evaluators("device",    evaluate_device_logic, 0);
  onex_set_evaluators("user",      evaluate_user, 0);
  onex_set_evaluators("battery",   evaluate_battery_in, 0);
  onex_set_evaluators("bluetooth", evaluate_bluetooth_in, 0);
  onex_set_evaluators("touch",     evaluate_touch_in, 0);
  onex_set_evaluators("motion",    evaluate_motion_in, 0);
  onex_set_evaluators("button",    evaluate_button_in, 0);
  onex_set_evaluators("about",     evaluate_about_in, 0);
  onex_set_evaluators("backlight", evaluate_edit_rule, evaluate_light_logic, evaluate_backlight_out, 0);
  onex_set_evaluators("clock",     evaluate_clock_sync, evaluate_clock, 0);
  onex_set_evaluators("editable",  evaluate_edit_rule, 0);

  object_set_evaluator(onex_device_object, "device");

  user     =object_new(0, "user",      "user", 8);
  battery  =object_new(0, "battery",   "battery", 4);
  bluetooth=object_new(0, "bluetooth", "bluetooth", 4);
  touch    =object_new(0, "touch",     "touch", 6);
  motion   =object_new(0, "motion",    "motion", 8);
  button   =object_new(0, "button",    "button", 4);
  backlight=object_new(0, "backlight", "light editable", 9);
  oclock   =object_new(0, "clock",     "clock event", 12);
  watchface=object_new(0, "editable",  "watchface editable", 6);
  viewlist =object_new(0, "editable",  "list editable", 4);
  home     =object_new(0, "default",   "home", 4);
  calendar =object_new(0, "editable",  "event list editable", 4);
  about    =object_new(0, "about",     "about", 4);

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
  viewlistuid =object_property(viewlist, "UID");
  homeuid     =object_property(home, "UID");
  calendaruid =object_property(calendar, "UID");
  aboutuid    =object_property(about, "UID");

  object_property_set(backlight, "light", "on");
  object_property_set(backlight, "level", "high");
  object_property_set(backlight, "timeout", "6000");
  object_property_set(backlight, "touch", touchuid);
  object_property_set(backlight, "motion", motionuid);
  object_property_set(backlight, "button", buttonuid);

  object_property_set(oclock, "title", "OnexOS Clock");
  object_property_set(oclock, "ts", "%unknown");
  object_property_set(oclock, "tz", "%unknown");
  object_property_set(oclock, "device", deviceuid);

  object_property_set(watchface, "clock", clockuid);
  object_property_set(watchface, "ampm-24hr", "ampm");

  object_property_add(viewlist, (char*)"list", homeuid);
  object_property_add(viewlist, (char*)"list", calendaruid);
  object_property_add(viewlist, (char*)"list", aboutuid);

  object_property_set(home, (char*)"battery",   batteryuid);
  object_property_set(home, (char*)"bluetooth", bluetoothuid);
  object_property_set(home, (char*)"watchface", watchfaceuid);

  object_property_set(user, "viewing", viewlistuid);

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
  onex_run_evaluators(aboutuid, 0);

  time_ticker(every_5ms,        5);
  time_ticker(every_second,  1000);
  time_ticker(every_10s,    10000);

  while(1){

    uint64_t ct=time_ms();

#if defined(LOG_LOOP_TIME)
    static uint64_t lt=0;
    if(lt) log_write("loop time %ldms", (uint32_t)(ct-lt));
    lt=ct;
#endif

    if(!onex_loop()){
      gpio_sleep(); // will gpio_wake() when ADC read
      spi_sleep();  // will spi_wake() as soon as spi_tx called
      i2c_sleep();  // will i2c_wake() in irq to read values
      // stop every_5ms timer!!
      boot_sleep();
    }

    if(event_tick_5ms){
      event_tick_5ms=false;
      lv_task_handler();
    }

#if defined(LOG_TO_GFX)
    if(event_log_buffer){
      draw_log();
      event_log_buffer=0;
    }
#endif

    static uint64_t feeding_time=0;
    if(ct>feeding_time && gpio_get(BUTTON_1)!=BUTTONS_ACTIVE_STATE){
      boot_feed_watchdog();
      feeding_time=ct+1000;
    }
  }
}

// ------------ LVGL ----------------------------

lv_disp_drv_t* disp_for_flush_ready=0;

void area_drawn()
{
  lv_disp_flush_ready(disp_for_flush_ready);
}

void initiate_display_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
  disp_for_flush_ready=disp;
  display_draw_area(area->x1, area->x2, area->y1, area->y2, (uint16_t*)color_p, area_drawn);
}

static bool read_touch(lv_indev_drv_t* indev, lv_indev_data_t* data)
{
  if(user_active && touch_info.action==TOUCH_ACTION_CONTACT){
    data->state = LV_INDEV_STATE_PR;
    touch_info=touch_get_info();
  }
  else{
    data->state = LV_INDEV_STATE_REL;
  }
  data->point.x=touch_info.x;
  data->point.y=touch_info.y;
  return false;
}

static lv_disp_buf_t disp_buf;

#define LV_BUF_SIZE (LV_HOR_RES_MAX * 6)
static lv_color_t lv_buf1[LV_BUF_SIZE];
static lv_color_t lv_buf2[LV_BUF_SIZE];

void log_lv(signed char ch,  const char * st, long unsigned int in,  const char * st2)
{
  log_write("log_lv: [%d] [%s] [%ld] [%s]", ch, st, in, st2);
}

static lv_style_t screen_style;

void init_lv()
{
  lv_init();
  lv_log_register_print_cb(log_lv);

  lv_disp_buf_init(&disp_buf, lv_buf1, lv_buf2, LV_BUF_SIZE);

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = initiate_display_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = read_touch;
  lv_indev_drv_register(&indev_drv);

  lv_style_copy(&screen_style, &lv_style_plain);
  screen_style.body.main_color = LV_COLOR_BLACK;
  screen_style.body.grad_color = LV_COLOR_BLACK;
  screen_style.text.color      = LV_COLOR_WHITE;
  screen_style.image.color     = LV_COLOR_WHITE;
}

// ------------------- evaluators ----------------

bool evaluate_default(object* o, void* d)
{
  log_write("evaluate_default data=%p\n", d); object_log(o);
  return true;
}

#define BATTERY_ZERO_PERCENT 3400
#define BATTERY_100_PERCENT 4000
#define BATTERY_PERCENT_STEPS 2
bool evaluate_battery_in(object* o, void* d)
{
  int32_t bv = gpio_read(ADC_CHANNEL);
  int32_t mv = bv*2000/(1024/(33/10));
  int8_t pc = ((mv-BATTERY_ZERO_PERCENT)*100/((BATTERY_100_PERCENT-BATTERY_ZERO_PERCENT)*BATTERY_PERCENT_STEPS))*BATTERY_PERCENT_STEPS;
  if(pc<0) pc=0;
  if(pc>100) pc=100;
  snprintf(buf, 16, "%d%%(%ld)", pc, mv);

  object_property_set(battery, "percent", buf);

  int batt=gpio_get(CHARGE_SENSE);
  snprintf(buf, 16, "%s", batt? "powering": "charging");
  object_property_set(battery, "status", buf);

  return true;
}

bool evaluate_bluetooth_in(object* o, void* d)
{
  object_property_set(bluetooth, "connected", ble_info.connected? "yes": "no");
  snprintf(buf, 16, "%3d", ble_info.rssi);
  object_property_set(bluetooth, "rssi", buf);
  return true;
}

bool evaluate_touch_in(object* o, void* d)
{
  snprintf(buf, 64, "%3d %3d", touch_info.x, touch_info.y);
  object_property_set(touch, "coords", buf);

  snprintf(buf, 64, "%s %s", touch_actions[touch_info.action], touch_gestures[touch_info.gesture]);
  object_property_set(touch, "action", buf);

  snprintf(buf, 64, "%d", touch_info_stroke);
  object_property_set(touch, "stroke", buf);

  return true;
}

bool evaluate_motion_in(object* o, void* d)
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

  snprintf(buf, 64, "%d %d %d %d", motion_info.x, motion_info.y, motion_info.z, motion_info.m);
  object_property_set(motion, "x-y-z-m", buf);
  object_property_set(motion, "gesture", viewscreen? "view-screen": "none");

  return true;
}

bool evaluate_button_in(object* o, void* d)
{
  bool button_pressed=(gpio_get(BUTTON_1)==BUTTONS_ACTIVE_STATE);
  object_property_set(button, "state", button_pressed? "down": "up");
  return true;
}

extern char __BUILD_TIMESTAMP;
extern char __BOOTLOADER_NUMBER;

bool evaluate_about_in(object* o, void* d)
{
  snprintf(buf, 32, "%lu %lu", (unsigned long)&__BUILD_TIMESTAMP, (unsigned long)&__BOOTLOADER_NUMBER);
  object_property_set(about, "build-info", buf);

  snprintf(buf, 16, "%d%%", boot_cpu());
  object_property_set(about, "cpu", buf);

  return true;
}

bool evaluate_backlight_out(object* o, void* d)
{
  bool light_on=object_property_is(backlight, "light", "on");

  if(light_on && !user_active){

    display_wake();

    bool mid =object_property_is(backlight, "level", "mid");
    bool high=object_property_is(backlight, "level", "high");
    gpio_set(LCD_BACKLIGHT_LOW,               LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  (mid||high)? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_HIGH, (high)?      LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);

  //touch_wake();

    user_active=true;

    onex_run_evaluators(useruid, 0);
  }
  else
  if(!light_on && user_active){

    user_active=false;

  //touch_sleep();

    gpio_set(LCD_BACKLIGHT_LOW,  !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_HIGH, !LEDS_ACTIVE_STATE);

    display_sleep();
  }
  return true;
}

// -------------------- User --------------------------

static void draw_by_type(char* path, bool touchevent);
static void draw_home(char* path);
static void draw_calendar(char* path);
static void draw_event(char* path);
static void draw_about(char* path);
static void draw_list(char* p, bool touchevent);
static void draw_default(char* path);

bool evaluate_user(object* o, void* touchevent)
{
  if(user_active) draw_by_type("viewing", !!touchevent);
  return true;
}

static char pi[64];
static char pl[64];

void draw_by_type(char* p, bool touchevent)
{
  snprintf(pi, 32, "%s:is", p);

  if(object_property_contains(user, pi, "home"))  draw_home(p);               else
  if(object_property_contains(user, pi, "event") &&
     object_property_contains(user, pi, "list") ) draw_calendar(p);           else
  if(object_property_contains(user, pi, "event")) draw_event(p);              else
  if(object_property_contains(user, pi, "about")) draw_about(p);              else
  if(object_property_contains(user, pi, "list"))  draw_list(p, !!touchevent); else
                                                  draw_default(p);
}

static uint8_t list_index=1;

void draw_list(char* p, bool touchevent)
{
  if(touchevent){
    if(touch_info.gesture==TOUCH_GESTURE_LEFT  && touch_info_stroke > 50){
      list_index++;
    }
    else
    if(touch_info.gesture==TOUCH_GESTURE_RIGHT && touch_info_stroke > 50){
      list_index--;
    }
  }
  uint8_t list_len=object_property_length(user, "viewing:list");
  if(list_index<1       ) list_index=list_len;
  if(list_index>list_len) list_index=1;

  snprintf(pl, 32, "%s:list:%d", p, list_index); // all goes wrong if this recurses (list:list) !

  draw_by_type(pl, false);
}

static lv_obj_t* home_screen;

static lv_obj_t* time_label;
static lv_obj_t* date_label;
static lv_obj_t* battery_level;
static lv_obj_t* ble_rssi;

static lv_style_t time_label_style;
static lv_style_t meter_bg_style;
static lv_style_t battery_level_style;
static lv_style_t ble_rssi_style;

#define BATTERY_LOW      LV_COLOR_RED
#define BATTERY_MED      LV_COLOR_ORANGE
#define BATTERY_HIGH     LV_COLOR_GREEN
#define BATTERY_CHARGING LV_COLOR_WHITE

#define BLE_CONNECTED    LV_COLOR_BLUE
#define BLE_DISCONNECTED LV_COLOR_GRAY

void draw_home(char* path)
{
  if(!home_screen){
    home_screen  = lv_obj_create(0,0);
    lv_label_set_style(home_screen, LV_LABEL_STYLE_MAIN, &screen_style);

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

    battery_level = lv_bar_create(home_screen, 0);
    lv_obj_set_size(battery_level, 235, 4);
    lv_obj_align(battery_level, home_screen, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    ble_rssi = lv_bar_create(home_screen, 0);
    lv_obj_set_size(ble_rssi, 235, 4);
    lv_obj_align(ble_rssi, home_screen, LV_ALIGN_IN_TOP_LEFT, 0, 6);

    lv_style_copy(&time_label_style, &screen_style);
    time_label_style.text.font= &noto_sans_numeric_80;
    lv_label_set_style(time_label, LV_LABEL_STYLE_MAIN, &time_label_style);

    lv_style_copy(&meter_bg_style, &screen_style);
    meter_bg_style.body.radius=0;
    meter_bg_style.body.main_color=LV_COLOR_GRAY;
    meter_bg_style.body.grad_color=LV_COLOR_GRAY;

    lv_style_copy(&battery_level_style, &screen_style);
    battery_level_style.body.radius = 0;
    battery_level_style.body.padding.top = 0;
    battery_level_style.body.padding.bottom = 0;
    battery_level_style.body.padding.left = 0;
    battery_level_style.body.padding.right = 0;
    battery_level_style.body.main_color = LV_COLOR_GREEN;
    battery_level_style.body.grad_color = LV_COLOR_GREEN;

    lv_style_copy(&ble_rssi_style, &battery_level_style);
    ble_rssi_style.body.main_color = LV_COLOR_BLUE;
    ble_rssi_style.body.grad_color = LV_COLOR_BLUE;

    lv_bar_set_style(battery_level, LV_BAR_STYLE_INDIC, &battery_level_style);
    lv_bar_set_style(battery_level, LV_BAR_STYLE_BG,    &meter_bg_style);
    lv_bar_set_style(ble_rssi,      LV_BAR_STYLE_INDIC, &ble_rssi_style);
    lv_bar_set_style(ble_rssi,      LV_BAR_STYLE_BG,    &meter_bg_style);
  }
  if(lv_scr_act()!=home_screen){
    lv_scr_load(home_screen);
  }
  snprintf(buf, 64, "%s:battery:percent", path);      char* pc=object_property(   user, buf);
  snprintf(buf, 64, "%s:battery:status", path);       bool  ch=object_property_is(user, buf, "charging");
  snprintf(buf, 64, "%s:bluetooth:connected", path);  bool  bl=object_property_is(user, buf, "yes");
  snprintf(buf, 64, "%s:watchface:clock:ts", path);   char* ts=object_property(   user, buf);
  snprintf(buf, 64, "%s:watchface:clock:tz:2", path); char* tz=object_property(   user, buf);
  snprintf(buf, 64, "%s:watchface:ampm-24hr", path);  bool h24=object_property_is(user, buf, "24hr");

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
  if(pcnum<0) pcnum=0;
  if(pcnum>100) pcnum=100;

  lv_color_t batt_col;
  if(ch)       batt_col=BATTERY_CHARGING; else
  if(pcnum>67) batt_col=BATTERY_HIGH;     else
  if(pcnum>33) batt_col=BATTERY_MED;
  else         batt_col=BATTERY_LOW;

  int8_t blnum=100;
  lv_color_t ble_col=bl? BLE_CONNECTED: BLE_DISCONNECTED;

  battery_level_style.body.main_color=batt_col;
  battery_level_style.body.grad_color=batt_col;
  lv_obj_refresh_style(battery_level);
  lv_bar_set_value(battery_level, pcnum, LV_ANIM_OFF);

  ble_rssi_style.body.main_color=ble_col;
  ble_rssi_style.body.grad_color=ble_col;
  lv_obj_refresh_style(ble_rssi);
  lv_bar_set_value(ble_rssi, blnum, LV_ANIM_OFF);
}

static lv_obj_t* calendar_screen;

static lv_obj_t* calendar_title;
static lv_obj_t* len_label;
static lv_obj_t* plus_button;
static lv_obj_t* plus_button_label;

static void plus_button_event(lv_obj_t * obj, lv_event_t event)
{
  if(event==LV_EVENT_CLICKED) {
    log_write("Plus button pressed\n");
  }
}

void draw_calendar(char* path)
{
  if(!calendar_screen){
    calendar_screen = lv_obj_create(0,0);
    lv_label_set_style(calendar_screen, LV_LABEL_STYLE_MAIN, &screen_style);

    calendar_title=lv_label_create(calendar_screen, 0);
    lv_label_set_long_mode(calendar_title, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(calendar_title, 200);
    lv_obj_set_height(calendar_title, 200);
    lv_label_set_align(calendar_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(calendar_title, calendar_screen, LV_ALIGN_CENTER, -5, -50);

    len_label=lv_label_create(calendar_screen, 0);
    lv_label_set_long_mode(len_label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(len_label, 150);
    lv_obj_set_height(len_label, 100);
    lv_label_set_align(len_label, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(len_label, calendar_screen, LV_ALIGN_IN_TOP_LEFT, 50, 160);

    plus_button = lv_btn_create(calendar_screen, 0);
    lv_obj_set_event_cb(plus_button, plus_button_event);
    lv_obj_align(plus_button, 0, LV_ALIGN_CENTER, 0, -40);

    plus_button_label = lv_label_create(plus_button, 0);
    lv_label_set_text(plus_button_label, "+");
  }
  if(lv_scr_act()!=calendar_screen){
    lv_scr_load(calendar_screen);
  }
  snprintf(buf, 64, "%s:title", path); char* title=object_property_values(user, buf);
  snprintf(buf, 64, "%s:list",  path); int n=object_property_length(user, buf);

  snprintf(buf, 64, "len: (%d)", n);

  lv_label_set_text(calendar_title, title? title: "Calendar");
  lv_label_set_text(len_label, buf);
}

void draw_event(char* path)
{
}

static lv_obj_t* about_screen;

static lv_obj_t* about_title;
static lv_obj_t* build_label;
static lv_obj_t* cpu_label;
static lv_obj_t* log_label;

static lv_style_t log_label_style;
static lv_style_t build_label_style;

void draw_about(char* path)
{
  if(!about_screen){
    about_screen = lv_obj_create(0,0);
    lv_label_set_style(about_screen, LV_LABEL_STYLE_MAIN, &screen_style);

    about_title=lv_label_create(about_screen, 0);
    lv_label_set_long_mode(about_title, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(about_title, 200);
    lv_obj_set_height(about_title, 200);
    lv_label_set_align(about_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(about_title, about_screen, LV_ALIGN_CENTER, -5, -50);

    cpu_label=lv_label_create(about_screen, 0);
    lv_label_set_long_mode(cpu_label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(cpu_label, 50);
    lv_obj_set_height(cpu_label, 100);
    lv_label_set_align(cpu_label, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(cpu_label, about_screen, LV_ALIGN_IN_TOP_LEFT, 50, 160);

    build_label=lv_label_create(about_screen, 0);
    lv_label_set_long_mode(build_label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(build_label, 230);
    lv_obj_set_height(build_label, 100);
    lv_label_set_align(build_label, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(build_label, about_screen, LV_ALIGN_IN_TOP_LEFT, 50, 200);

    log_label=lv_label_create(about_screen, 0);
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(log_label, 230);
    lv_obj_set_height(log_label, 100);
    lv_label_set_align(log_label, LV_LABEL_ALIGN_LEFT);
    lv_obj_align(log_label, about_screen, LV_ALIGN_IN_TOP_LEFT, 5, 215);

    lv_label_set_text(log_label, "");

    lv_style_copy(&log_label_style, &screen_style);
    log_label_style.text.font= &lv_font_roboto_12;
    log_label_style.text.color= LV_COLOR_TEAL;
    lv_label_set_style(log_label, LV_LABEL_STYLE_MAIN, &log_label_style);

    lv_style_copy(&build_label_style, &screen_style);
    build_label_style.text.font= &lv_font_roboto_12;
    build_label_style.text.color= LV_COLOR_ORANGE;
    lv_label_set_style(build_label, LV_LABEL_STYLE_MAIN, &build_label_style);

    lv_label_set_style(cpu_label, LV_LABEL_STYLE_MAIN, &build_label_style);
  }
  if(lv_scr_act()!=about_screen){
    lv_scr_load(about_screen);
  }
  snprintf(buf, 64, "%s:build-info", path); char* bnf=object_property_values(user, buf);
  snprintf(buf, 64, "%s:cpu", path);        char* cpu=object_property(       user, buf);

  lv_label_set_text(about_title, "About device");

  lv_label_set_text(build_label, bnf);

  lv_label_set_text(cpu_label, cpu);

//boot_dfu_start();
}

static lv_obj_t* default_screen;

static lv_obj_t* is_label;

static lv_style_t is_label_style;

void draw_default(char* path)
{
  if(!default_screen){
    default_screen = lv_obj_create(0,0);
    lv_label_set_style(default_screen, LV_LABEL_STYLE_MAIN, &screen_style);

    is_label=lv_label_create(default_screen, 0);
    lv_label_set_long_mode(is_label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(is_label, 200);
    lv_obj_set_height(is_label, 200);
    lv_label_set_align(is_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(is_label, default_screen, LV_ALIGN_CENTER, -5, -50);

    lv_style_copy(&is_label_style, &screen_style);
    is_label_style.text.color= LV_COLOR_TEAL;
    lv_label_set_style(is_label, LV_LABEL_STYLE_MAIN, &is_label_style);
  }
  if(lv_scr_act()!=default_screen){
    lv_scr_load(default_screen);
  }

  snprintf(buf, 64, "%s:is", path); char* is=object_property(user, buf);

  lv_label_set_text(is_label, is);
}

#if defined(LOG_TO_GFX)
void draw_log()
{
  lv_label_set_text(log_label, (const char*)event_log_buffer);
}
#endif

