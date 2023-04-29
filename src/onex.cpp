
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>
#if defined(__ANDROID__)
#include <android/log.h>
#define log_init()
#define log_loop() true
#undef  log_write
#define log_write(...) ((void)__android_log_print(ANDROID_LOG_INFO, "OnexApp", __VA_ARGS__))
#define log_flush()
#endif
#include <onex-kernel/mem.h>
#include <onn.h>
#include <onr.h>
}

object* config;

object* user;
object* responses;
object* bluetooth;
object* taglookup;

char* userUID=0;
char* responsesUID=0;
char* homeUID=0;
char* clockUID=0;
char* bluetoothUID=0;
char* buttonUID=0;
char* taglookupUID;

extern bool evaluate_default(object* o, void* d);
extern bool evaluate_user(object* o, void* d);
extern bool evaluate_event(object* o, void* d);
static bool evaluate_bluetooth_in(object* o, void* d);
static bool evaluate_bluetooth_out(object* o, void* d);

static void every_second(){ onex_run_evaluators(clockUID, 0); }

extern "C" void sprintExternalStorageDirectory(char* buf, int buflen, const char* format);
extern "C" void ensureBluetoothConnecting();

static char* ble_state=0;
static char* ble_mac=0;

char* init_onex()
{
  onex_set_evaluators((char*)"editable",                         evaluate_edit_rule, 0);
  onex_set_evaluators((char*)"default",                          evaluate_edit_rule, evaluate_default, 0);
  onex_set_evaluators((char*)"device",                                               evaluate_device_logic, 0);
  onex_set_evaluators((char*)"user",                                                 evaluate_user, 0);
  onex_set_evaluators((char*)"clock",                            evaluate_edit_rule, evaluate_clock, 0);
  onex_set_evaluators((char*)"event",                            evaluate_edit_rule, evaluate_event, 0);
  onex_set_evaluators((char*)"light",                            evaluate_edit_rule, evaluate_light_logic, 0);
  onex_set_evaluators((char*)"bluetooth", evaluate_bluetooth_in, evaluate_edit_rule,                       evaluate_bluetooth_out, 0);

#if defined(__ANDROID__)
  char dbpath[128];
  sprintExternalStorageDirectory(dbpath, 128, "%s/Onex/onex.ondb");
  onex_init(dbpath);
#else
  onex_init((char*)"Onex/onex.ondb");
#endif

  config=onex_get_from_cache((char*)"uid-0");

  if(!config){
    // UTF-8 hex:  "\xF0\x9F\x98\x83  \xF0\x9F\x93\xA6"

    object* tagbirth=object_new_from((char*)"is: tag  title: birthday  icon: 📦  colour: red", 5);
    object* tagparty=object_new_from((char*)"is: tag  title: party     icon: 🎉  colour: yellow", 5);
    object* tagtrain=object_new_from((char*)"is: tag  title: train     icon: 🚆  colour: blue", 5);
    object* tagceleb=object_new_from((char*)"is: tag  title: celebrate icon: 🎉  colour: red", 5);
    object* taglove =object_new_from((char*)"is: tag  title: love      icon: 😍  colour: red", 5);

    taglookup=object_new_from((char*)"is: tag lookup", 100);
    object_property_set(taglookup, (char*)"birthday",  object_property(tagbirth, (char*)"UID"));
    object_property_set(taglookup, (char*)"party",     object_property(tagparty, (char*)"UID"));
    object_property_set(taglookup, (char*)"train",     object_property(tagtrain, (char*)"UID"));
    object_property_set(taglookup, (char*)"celebrate", object_property(tagceleb, (char*)"UID"));
    object_property_set(taglookup, (char*)"love",      object_property(taglove,  (char*)"UID"));
    taglookupUID=object_property(taglookup, (char*)"UID");
/*
    object* links=object_new(0, (char*)"default", (char*)"links list", 4);
    object_property_set(links, (char*)"list", object_property(taglookup, (char*)"UID"));
*/
    user=object_new(0, (char*)"user", (char*)"user", 8);
    userUID=object_property(user, (char*)"UID");

    responses=object_new(0, (char*)"default", (char*)"user responses", 12);
    char* responsesUID=object_property(responses, (char*)"UID");

    object* home=object_new(0, (char*)"editable",  (char*)"list editable", 4);
    homeUID=object_property(home, (char*)"UID");

    object* oclock=object_new(0, (char*)"clock", (char*)"clock event", 12);
    object_property_set(oclock, (char*)"title", (char*)"OnexApp Clock");
    clockUID=object_property(oclock, (char*)"UID");
    object_set_persist(oclock, (char*)"none");

    bluetooth=object_new(0, (char*)"bluetooth", (char*)"bluetooth", 6);
    object_property_set(bluetooth, (char*)"state", (char*)"BLE disconnected");
    bluetoothUID=object_property(bluetooth, (char*)"UID");

    object* button=object_new(0, (char*)"editable", (char*)"button", 6);
    object_property_set(button, (char*)"state", (char*)"up");
    buttonUID=object_property(button, (char*)"UID");

    object_set_evaluator(onex_device_object, (char*)"device");
    char* deviceUID=object_property(onex_device_object, (char*)"UID");

    object_property_add(onex_device_object, (char*)"user", userUID);
    object_property_add(onex_device_object, (char*)"io", clockUID);
    object_property_add(onex_device_object, (char*)"io", bluetoothUID);
    object_property_add(onex_device_object, (char*)"io", buttonUID);

    object_property_set(user, (char*)"responses", responsesUID);

    config=object_new((char*)"uid-0", 0, (char*)"config", 10);
    object_property_set(config, (char*)"user",      userUID);
    object_property_set(config, (char*)"responses", responsesUID);
    object_property_set(config, (char*)"clock",     clockUID);
    object_property_set(config, (char*)"bluetooth", bluetoothUID);
    object_property_set(config, (char*)"taglookup", taglookupUID);

    object_property_add(home, (char*)"list", clockUID);
    object_property_add(home, (char*)"list", userUID);
    object_property_add(home, (char*)"list", deviceUID);

    object_property_set(user, (char*)"viewing-l", homeUID);
  }
  else{
    userUID=     object_property(config, (char*)"user");
    responsesUID=object_property(config, (char*)"responses");
    clockUID=    object_property(config, (char*)"clock");
    bluetoothUID=object_property(config, (char*)"bluetooth");
    taglookupUID=object_property(config, (char*)"taglookup");

    user     =onex_get_from_cache(userUID);
    responses=onex_get_from_cache(responsesUID);
    bluetooth=onex_get_from_cache(bluetoothUID);
    taglookup=onex_get_from_cache(taglookupUID);

    ble_mac=mem_strdup(object_property(bluetooth, (char*)"mac"));
  }
  time_ticker(every_second, 1000);

  return ble_mac;
}

void loop_onex()
{
  while(true){
    if(!onex_loop()){
      time_delay_ms(5);
    }
  }
}

void on_alarm_recv(char* uid)
{
  onex_run_evaluators(uid, 0);
}

void connection_state(char* st)
{
  mem_free(ble_state);
  ble_state=mem_strdup(st);
  log_write("connection state=%s\n", ble_state);
  onex_run_evaluators(bluetoothUID, 0);
}

void set_ble_mac(char* bm)
{
  mem_free(ble_mac);
  ble_mac=mem_strdup(bm);
  onex_run_evaluators(bluetoothUID, 0);
}

bool evaluate_bluetooth_in(object* o, void* d)
{
  if(ble_state) object_property_set(bluetooth, (char*)"state", ble_state);
  if(ble_mac)   object_property_set(bluetooth, (char*)"mac",   ble_mac);
  return true;
}

bool evaluate_bluetooth_out(object* o, void* d)
{
#if defined(__ANDROID__)
  if(object_property_is(bluetooth, (char*)"state:2", (char*)"reconnecting")){
    mem_free(ble_state);
    ble_state=mem_strdup("BLE reconnecting");
    ensureBluetoothConnecting();
  }
#endif
  return true;
}

