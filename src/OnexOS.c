
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <boards.h>
#include <nrf_pwr_mgmt.h>
#include <nrf_bootloader_info.h>
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

object* user;
object* battery;
object* touch;
object* button;
object* backlight;
object* oclock;
object* watchface;
object* home;

char* deviceuid;
char* useruid;
char* batteryuid;
char* touchuid;
char* buttonuid;
char* backlightuid;
char* clockuid;
char* watchfaceuid;
char* homeuid;

static volatile bool event_tick_10ms=false;
static volatile bool event_tick_sec=false;
static volatile bool event_tick_min=false;
static volatile bool event_touch=false;
static volatile bool event_button=false;

static volatile touch_info_t touch_info;
static volatile bool         button_pressed;

static void every_10ms(){             event_tick_10ms=true; }
static void every_second(){           event_tick_sec=true; }
static void every_minute(){           event_tick_min=true; }
static void touched(touch_info_t ti){ event_touch=true;  touch_info=ti; }
static void button_changed(int p){    event_button=true; button_pressed=p; }

static bool evaluate_user(object* o, void* d);
static bool evaluate_battery_io(object* o, void* d);
static bool evaluate_touch_io(object* o, void* d);
static bool evaluate_button_io(object* o, void* d);
static bool evaluate_backlight_io(object* o, void* d);

static void draw_ui();
//static void draw_obj();

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

static void init_lv();

int main()
{
  log_init();
  time_init();
  gpio_init();
  blenus_init(0);

  init_lv();

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

  gpio_mode(LCD_BACKLIGHT_LOW, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_MID, OUTPUT);
  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);

  onex_init("");

  onex_set_evaluators("device",    evaluate_device_logic, 0);
  onex_set_evaluators("user",      evaluate_user, 0);
  onex_set_evaluators("battery",   evaluate_battery_io, 0);
  onex_set_evaluators("touch",     evaluate_touch_io, 0);
  onex_set_evaluators("button",    evaluate_button_io, 0);
  onex_set_evaluators("backlight", evaluate_edit_rule, evaluate_light_logic, evaluate_backlight_io, 0);
  onex_set_evaluators("clock",     evaluate_clock_sync, evaluate_clock, 0);
  onex_set_evaluators("editable",  evaluate_edit_rule, 0);

  object_set_evaluator(onex_device_object, "device");

  user     =object_new(0, "user",      "user", 8);
  battery  =object_new(0, "battery",   "battery", 8);
  touch    =object_new(0, "touch",     "touch", 8);
  button   =object_new(0, "button",    "button", 4);
  backlight=object_new(0, "backlight", "editable light", 7);
  oclock   =object_new(0, "clock",     "clock event", 12);
  watchface=object_new(0, "editable",  "editable watchface", 6);
  home     =object_new(0, "editable",  "editable", 4);

  deviceuid   =object_property(onex_device_object, "UID");
  useruid     =object_property(user, "UID");
  batteryuid  =object_property(battery, "UID");
  touchuid    =object_property(touch, "UID");
  buttonuid   =object_property(button, "UID");
  backlightuid=object_property(backlight, "UID");
  clockuid    =object_property(oclock, "UID");
  watchfaceuid=object_property(watchface, "UID");
  homeuid     =object_property(home, "UID");

  object_property_set(backlight, "light", "on");
  object_property_set(backlight, "level", "high");
  object_property_set(backlight, "timeout", "4000");
  object_property_set(backlight, "touch", touchuid);
  object_property_set(backlight, "button", buttonuid);

  object_property_set(oclock, "title", "OnexOS Clock");
  object_property_set(oclock, "ts", "%unknown");
  object_property_set(oclock, "tz", "%unknown");
  object_property_set(oclock, "device", deviceuid);

  object_property_set(watchface, "clock", clockuid);
  object_property_set(watchface, "ampm-24hr", "ampm");

  object_property_set(home, (char*)"battery", batteryuid);
  object_property_set(home, (char*)"watchface", watchfaceuid);

  object_property_set(user, "viewing", homeuid);

  object_property_add(onex_device_object, (char*)"user", useruid);
  object_property_add(onex_device_object, (char*)"io",   batteryuid);
  object_property_add(onex_device_object, (char*)"io",   touchuid);
  object_property_add(onex_device_object, (char*)"io",   buttonuid);
  object_property_add(onex_device_object, (char*)"io",   backlightuid);
  object_property_add(onex_device_object, (char*)"io",   clockuid);

  onex_run_evaluators(useruid, 0);
  onex_run_evaluators(batteryuid, 0);
  onex_run_evaluators(clockuid, 0);
  onex_run_evaluators(backlightuid, 0);

  time_ticker(every_10ms,      10);
  time_ticker(every_second,  1000);
  time_ticker(every_minute, 60000);

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
      onex_run_evaluators(batteryuid, 0);
    }
    if(event_touch){
      event_touch=false;
      onex_run_evaluators(touchuid, 0);
    }
    if(event_button){
      event_button=false;
      onex_run_evaluators(buttonuid, 0);
    }
    if(event_log_buffer){
      draw_log((char*)event_log_buffer);
      event_log_buffer=0;
    }
  }
}

bool evaluate_battery_io(object* o, void* d)
{
  char b[16];

  int16_t bv = gpio_read(ADC_CHANNEL);
  int16_t mv = bv*2000/(1024/(33/10));
  int8_t  pc = ((mv-3520)*100/5200)*10;
  snprintf(b, 16, "%d%%(%d)", pc, mv);

  object_property_set(battery, "percent", b);

  int batt=gpio_get(CHARGE_SENSE);
  snprintf(b, 16, "%s", batt? "battery": "charging");
  object_property_set(battery, "charge", b);

  return true;
}


bool evaluate_touch_io(object* o, void* d)
{
  char buf[64];
  snprintf(buf, 64, "%03d %03d", touch_info.x, touch_info.y);
  object_property_set(touch, "coords", buf);
  snprintf(buf, 64, "%s %s", touch_info.action==TOUCH_ACTION_CONTACT? "down": "up", touch_gestures[touch_info.gesture]);
  object_property_set(touch, "action", buf);
  return true;
}

bool evaluate_button_io(object* o, void* d)
{
  object_property_set(button, "state", button_pressed? "down": "up");
  // FOR TESTING
  if(button_pressed){
    sd_power_gpregret_clr(0, 0xffffffff);
    sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
  }
  // FOR TESTING
  return true;
}

bool evaluate_backlight_io(object* o, void* d)
{
  if(object_property_is(backlight, "light", "on")){
    bool mid =object_property_is(backlight, "level", "mid");
    bool high=object_property_is(backlight, "level", "high");
    gpio_set(LCD_BACKLIGHT_LOW,               LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  (mid||high)? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_HIGH, (high)?      LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
  } else {
    gpio_set(LCD_BACKLIGHT_LOW,  !LEDS_ACTIVE_STATE);
    gpio_set(LCD_BACKLIGHT_MID,  !LEDS_ACTIVE_STATE);
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

void init_lv()
{
  lv_init();
  lv_disp_buf_init(&disp_buf, lv_buf1, lv_buf2, LV_BUF_SIZE);
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = draw_area_and_ready;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  time_label=lv_label_create(lv_scr_act(), 0);
  lv_label_set_long_mode(time_label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(time_label, 240);
  lv_obj_set_height(time_label, 200);
  lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(time_label, lv_scr_act(), LV_ALIGN_CENTER, -5, -20);

  date_label=lv_label_create(lv_scr_act(), 0);
  lv_label_set_long_mode(date_label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(date_label, 200);
  lv_obj_set_height(date_label, 200);
  lv_label_set_align(date_label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(date_label, lv_scr_act(), LV_ALIGN_CENTER, -5, 50);

  lv_style_t bg;
  lv_style_copy(&bg, &lv_style_plain);
  bg.body.main_color = LV_COLOR_BLACK;
  bg.body.grad_color = LV_COLOR_BLACK;
  bg.text.color      = LV_COLOR_WHITE;
  bg.image.color     = LV_COLOR_WHITE;
  lv_label_set_style(lv_scr_act(), LV_LABEL_STYLE_MAIN, &bg);

  lv_style_t lb;
  lv_style_copy(&lb, &bg);
  lb.text.font= &noto_sans_numeric_80;
  lv_label_set_style(time_label, LV_LABEL_STYLE_MAIN, &lb);

  lv_label_set_text(time_label, "00:00");
  lv_label_set_text(date_label, "Onex");
}

extern char __BUILD_TIMESTAMP;
extern char __BOOTLOADER_NUMBER;

void draw_ui()
{
  char* pc=object_property(   user, "viewing:battery:percent");
  bool  ch=object_property_is(user, "viewing:battery:charge", "charging");
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

  strftime(t, 32, h24? "%H:%M": "%l:%M %p", &tms);
  lv_label_set_text(time_label, t);

  strftime(t, 32, h24? "24 %d %h": "%p %d %h", &tms);
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

  #define BATTERY_PAD 2
  #define BATTERY_WIDTH (SCREEN_WIDTH-2*BATTERY_PAD)
  gfx_rect_fill(BATTERY_PAD,0, BATTERY_PAD+ BATTERY_WIDTH,            2, GFX_GREY_3);
  gfx_rect_fill(BATTERY_PAD,0, BATTERY_PAD+(BATTERY_WIDTH*pcnum)/100, 2, batt_col);

  log_write((time_es()%2)? "%u %u\n%s/": "%u %u\n%s\\", (unsigned long)&__BOOTLOADER_NUMBER, (unsigned long)&__BUILD_TIMESTAMP, pc? pc: "-");
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

