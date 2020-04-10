
#include <boards.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onex-kernel/gpio.h>
#include <onex-kernel/blenus.h>
#include <onex-kernel/gfx.h>
#include <onex-kernel/touch.h>
#include <onf.h>
#include <onr.h>

object* sensors;
object* controllers;
object* user;

char* deviceuid;
char* sensorsuid;
char* controllersuid;
char* useruid;

void button_changed(int);
void every_minute();

static touch_info_t ti;
static bool new_touch_info=false;

void touched(touch_info_t touchinfo)
{
  ti=touchinfo;
  new_touch_info=true;
}

static bool evaluate_user(object* sensors, void* pressed);
static bool evaluate_sensors_io(object* sensors, void* pressed);
static bool evaluate_controllers_io(object* sensors, void* pressed);

static void draw_ui();

#define ADC_CHANNEL 0

void* x;
#define WHERESTHEHEAP(s) x = malloc(1); log_write("heap after %s: %x\n", s, x);

int main()
{
  log_init();
  time_init();
  gpio_init();
  blenus_init(0);

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

  object_set_evaluator(onex_device_object, "device");
  user       =object_new(0, "user",        "user", 8);
  sensors    =object_new(0, "sensors",     "sensors", 8);
  controllers=object_new(0, "controllers", "editable controllers", 4);

  deviceuid     =object_property(onex_device_object, "UID");
  useruid       =object_property(user, "UID");
  sensorsuid    =object_property(sensors, "UID");
  controllersuid=object_property(controllers, "UID");

  object_property_add(onex_device_object, (char*)"user", useruid);
  object_property_add(onex_device_object, (char*)"io",   sensorsuid);
  object_property_add(onex_device_object, (char*)"io",   controllersuid);

  object_property_set(user, "viewing", deviceuid);
  object_property_set(controllers, "backlight", "on");

  onex_run_evaluators(sensorsuid, false);
  onex_run_evaluators(controllersuid, 0);

  time_ticker(every_minute, 60000);

  draw_ui();

  while(1){

    onex_loop();

    if(new_touch_info){
      new_touch_info=false;
      onex_run_evaluators(controllersuid, (ti.gesture==TOUCH_GESTURE_TAP_LONG)? "off": "on");
    }
  }
}

void button_changed(int pressed)
{
  onex_run_evaluators(sensorsuid, pressed? "down": "up");
}

void every_minute()
{
  onex_run_evaluators(sensorsuid, 0);
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

void draw_ui()
{
  gfx_rect_line(0,0, SCREEN_WIDTH,SCREEN_HEIGHT, GFX_GREY_F, PADDING);

  gfx_screen_colour(ACTION_BG);
  gfx_rect_fill(PADDING,PADDING, SCREEN_WIDTH-PADDING,ROW_HEIGHT, ACTION_BG);
  gfx_pos(PADDING+L_PADDING, PADDING+T_PADDING);
  gfx_text_colour(ACTION_COLOUR);
  gfx_text("user");
}

