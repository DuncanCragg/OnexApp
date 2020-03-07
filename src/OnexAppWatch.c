
#include <stdlib.h>
#include <boards.h>
#include <onex-kernel/gpio.h>
#if defined(HAS_SERIAL)
#include <onex-kernel/serial.h>
#endif
#include <onex-kernel/blenus.h>
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#include <onf.h>
#include <onr.h>

object* button;
object* light;
char* buttonuid;
char* lightuid;

void button_changed(int);
bool evaluate_button_io(object* button, void* pressed);
bool evaluate_light_io(object* light, void* d);

void* x;
#define WHERESTHEHEAP(s) x = malloc(1); log_write("heap after %s: %x\n", s, x);

int main()
{
  log_init();
  time_init();
#if defined(HAS_SERIAL)
  serial_init(0,0);
#endif
  blenus_init(0);
  onex_init("");

#if defined(BOARD_PCA10059)
  gpio_mode_cb(BUTTON_1, INPUT_PULLUP, button_changed);
  gpio_mode(LED1_G, OUTPUT);
  gpio_mode(LED2_B, OUTPUT);
#elif defined(BOARD_PINETIME)
  gpio_mode_cb(BUTTON_1, INPUT_PULLDOWN, button_changed);
  gpio_mode(   BUTTON_ENABLE, OUTPUT);
  gpio_set(    BUTTON_ENABLE, 1);
  gpio_mode(LED_3, OUTPUT);
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
  gpio_set(LED1_G, 0);
  gpio_set(LED2_B, 1);
#elif defined(BOARD_PINETIME)
  gpio_set(LED_3, 1);
#endif

  while(1){
    onex_loop();
  }
}

void button_changed(int pressed)
{
  onex_run_evaluators(buttonuid, (void*)(bool)pressed);
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
    WHERESTHEHEAP("evaluate_light_io on");
#if defined(BOARD_PCA10059)
    gpio_set(LED2_B, 0);
#elif defined(BOARD_PINETIME)
    gpio_set(LED_3, 0);
#endif
  } else {
    WHERESTHEHEAP("evaluate_light_io off");
#if defined(BOARD_PCA10059)
    gpio_set(LED2_B, 1);
#elif defined(BOARD_PINETIME)
    gpio_set(LED_3, 1);
#endif
  }
  return true;
}

