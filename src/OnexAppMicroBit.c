
#include <variant.h>
#include <onex-kernel/gpio.h>
#include <onex-kernel/serial.h>
#include <onf.h>
#include <behaviours.h>

object* button;
object* light;

const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

bool evaluate_button_io(object* button, void* pressed);
bool evaluate_light_io(object* light, void* d);

int main()
{
  onex_init("");

  gpio_mode(BUTTON_1, INPUT_PULLUP);
  for(uint8_t l=0; l< LEDS_NUMBER; l++) gpio_mode(leds_list[l], OUTPUT);

  onex_set_evaluators("evaluate_button", evaluate_button_io, 0);
  onex_set_evaluators("evaluate_light", evaluate_light_logic, evaluate_light_io, 0);

  char* buttonuid = "uid-1-2-3";
  button=object_new(buttonuid, "evaluate_button", "button", 4);
  light =object_new(0,         "evaluate_light",  "light", 4);
  char* lightuid=object_property(light, "UID");

  serial_printf("Light UID %s\n", lightuid);

  object_property_set(button, "name", "£€§");

  object_property_set(light, "light", "off");
  object_property_set(light, "button", buttonuid);

  onex_run_evaluators(lightuid, 0);

  bool button_pressed=false;
  int todo=0;
  while(1){

    onex_loop();

    if(button_pressed != !gpio_get(BUTTON_1)){
      button_pressed = !gpio_get(BUTTON_1);
      onex_run_evaluators(buttonuid, (void*)button_pressed);
    }
  }
}

bool evaluate_button_io(object* button, void* pressed)
{
  char* s=(char*)(pressed? "down": "up");
  object_property_set(button, "state", s);
  return true;
}

bool evaluate_light_io(object* light, void* d)
{
  if(object_property_is(light, "light", "on")){ gpio_set(leds_list[3], 1); gpio_set(leds_list[10], 1); }
  else                                        { gpio_set(leds_list[3], 0); gpio_set(leds_list[10], 0); }
  return true;
}

