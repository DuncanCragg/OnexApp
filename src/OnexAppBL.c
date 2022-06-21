
#include <boards.h>
#include <onex-kernel/log.h>
#include <onex-kernel/time.h>
#include <onex-kernel/gpio.h>
#if defined(HAS_SERIAL)
#include <onex-kernel/serial.h>
#endif
#include <onex-kernel/blenus.h>
#if defined(BOARD_PINETIME)
#include <onex-kernel/gfx.h>
#include <onex-kernel/touch.h>
#endif
#include <onn.h>
#include <onr.h>

object* button;
object* light;
char* buttonuid;
char* lightuid;

static void button_changed(uint8_t pin, uint8_t type);

#if defined(BOARD_PINETIME)
static touch_info_t ti;
static bool new_touch_info=false;

void touched(touch_info_t touchinfo)
{
  ti=touchinfo;
  new_touch_info=true;
}
#endif

bool evaluate_button_io(object* button, void* pressed);
bool evaluate_light_io(object* light, void* d);

void* x;
#define WHERESTHEHEAP(s) x = malloc(1); log_write("heap after %s: %x\n", s, x);

int main()
{
  log_init();
  time_init();
  gpio_init();
#if defined(HAS_SERIAL)
  serial_init(0,0);
#endif
  blenus_init(0,0);

#if defined(BOARD_PINETIME)
  gfx_init();
  gfx_screen_colour(0xC618);
  gfx_text_colour(0x001F);
  gfx_screen_fill();
  touch_init(touched);
#endif

  onex_init("");

#if defined(BOARD_PCA10059)
  gpio_mode_cb(BUTTON_1, INPUT_PULLUP, RISING_AND_FALLING, button_changed);
  gpio_mode(LED1_G, OUTPUT);
  gpio_mode(LED2_B, OUTPUT);
#elif defined(BOARD_PINETIME)
  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, RISING_AND_FALLING, button_changed);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
  gpio_mode(LCD_BACKLIGHT_HIGH, OUTPUT);
#endif

  onex_set_evaluators("evaluate_button", evaluate_edit_rule, evaluate_button_io, 0);
  onex_set_evaluators("evaluate_light",  evaluate_edit_rule, evaluate_light_logic, evaluate_light_io, 0);
  onex_set_evaluators("evaluate_device", evaluate_device_logic, 0);

  button=object_new(0, "evaluate_button", "editable button", 4);
  light =object_new(0, "evaluate_light",  "editable light", 4);
  buttonuid=object_property(button, "UID");
  lightuid=object_property(light, "UID");

  object_property_set(button, "name", "£€§");

  object_property_set(light, "light", "off");

  object_set_evaluator(onex_device_object, (char*)"evaluate_device");
  object_property_add(onex_device_object, (char*)"io", buttonuid);
  object_property_add(onex_device_object, (char*)"io", lightuid);

  onex_run_evaluators(lightuid, 0);

#if defined(BOARD_PCA10059)
  gpio_set(LED1_G, LEDS_ACTIVE_STATE);
  gpio_set(LED2_B, !LEDS_ACTIVE_STATE);
#elif defined(BOARD_PINETIME)
  gfx_pos(10, 10);
  gfx_text("OnexOS");
  gpio_set(LCD_BACKLIGHT_HIGH, LEDS_ACTIVE_STATE);
#endif

  while(1){
    onex_loop();
#if defined(BOARD_PINETIME)
    if(new_touch_info){
      new_touch_info=false;
      if(ti.gesture==TOUCH_GESTURE_TAP_LONG){
        gpio_set(LCD_BACKLIGHT_HIGH, !LEDS_ACTIVE_STATE);
      }
      else {
        gpio_set(LCD_BACKLIGHT_HIGH, LEDS_ACTIVE_STATE);
      }
      button_changed(ti.action==TOUCH_ACTION_CONTACT);
    }
#endif
  }
}

static void button_changed(uint8_t pin, uint8_t type){
  bool button_pressed=(gpio_get(pin)==BUTTONS_ACTIVE_STATE);
#if defined(BOARD_PINETIME)
  gfx_pos(10, 110);
  gfx_text(button_pressed? "X": "O");
#endif
  onex_run_evaluators(buttonuid, (void*)(bool)button_pressed);
}

bool evaluate_button_io(object* button, void* pressed)
{
  char* s=(char*)(pressed? "down": "up");
  object_property_set(button, "state", s);
  return true;
}

bool evaluate_light_io(object* light, void* d)
{
  if(object_property_is(light, "light", "on")){
#if defined(BOARD_PCA10059)
    gpio_set(LED2_B, LEDS_ACTIVE_STATE);
#elif defined(BOARD_PINETIME)
    gfx_pos(10, 60);
    gfx_text("ON");
#endif
  } else {
#if defined(BOARD_PCA10059)
    gpio_set(LED2_B, !LEDS_ACTIVE_STATE);
#elif defined(BOARD_PINETIME)
    gfx_pos(10, 60);
    gfx_text("OFF");
#endif
  }
  return true;
}

