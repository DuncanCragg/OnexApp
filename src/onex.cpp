
#include <stdlib.h>

extern "C" {
#include <onex-kernel/time.h>
#include <onf.h>
#include <onr.h>
}

object* config;
object* user;
object* oclock;

char* userUID=0;
char* clockUID=0;

extern bool evaluate_default(object* o, void* d);
extern bool evaluate_user(object* o, void* d);
extern bool evaluate_event(object* o, void* d);

bool ticked=false;
static void every_second()
{
  ticked=true;
}

extern "C" void sprintExternalStorageDirectory(char* buf, int buflen, const char* format);

char* init_onex()
{
  char* blemac=0;

  onex_set_evaluators((char*)"default", evaluate_object_setter, evaluate_default, 0);
  onex_set_evaluators((char*)"device",  evaluate_device_logic, 0);
  onex_set_evaluators((char*)"user",                            evaluate_user, 0);
  onex_set_evaluators((char*)"clock",   evaluate_object_setter, evaluate_clock, 0);
  onex_set_evaluators((char*)"event",   evaluate_object_setter, evaluate_event, 0);
  onex_set_evaluators((char*)"light",   evaluate_object_setter, evaluate_light_logic, 0);

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

    object* taglookup=object_new_from((char*)"is: tag lookup", 100);
    object_property_set(taglookup, (char*)"birthday",  object_property(tagbirth, (char*)"UID"));
    object_property_set(taglookup, (char*)"party",     object_property(tagparty, (char*)"UID"));
    object_property_set(taglookup, (char*)"train",     object_property(tagtrain, (char*)"UID"));
    object_property_set(taglookup, (char*)"celebrate", object_property(tagceleb, (char*)"UID"));
    object_property_set(taglookup, (char*)"love",      object_property(taglove,  (char*)"UID"));
/*
    object* links=object_new(0, (char*)"default", (char*)"links list", 4);
    object_property_set(links, (char*)"list", object_property(taglookup, (char*)"UID"));
*/
    user=object_new(0, (char*)"user", (char*)"user", 8);
    userUID=object_property(user, (char*)"UID");

    oclock=object_new(0, (char*)"clock", (char*)"clock event", 12);
    object_property_set(oclock, (char*)"title", (char*)"OnexApp Clock");
    clockUID=object_property(oclock, (char*)"UID");

    object_set_evaluator(onex_device_object, (char*)"device");
    char* deviceUID=object_property(onex_device_object, (char*)"UID");

    object_property_add(onex_device_object, (char*)"user", userUID);
    object_property_add(onex_device_object, (char*)"io", clockUID);

    object_property_set(user, (char*)"viewing-l", deviceUID);

    config=object_new((char*)"uid-0", 0, (char*)"config", 10);
    object_property_set(config, (char*)"user", userUID);
    object_property_set(config, (char*)"clock", clockUID);
    object_property_set(config, (char*)"device", deviceUID);
    object_property_set(config, (char*)"taglookup", object_property(taglookup, (char*)"UID"));
  }
  else{
    userUID=object_property(config, (char*)"user");
    clockUID=object_property(config, (char*)"clock");
    blemac=object_property(config, (char*)"blemac");
    user=onex_get_from_cache(userUID);
    oclock=onex_get_from_cache(clockUID);
  }
  time_ticker(every_second, 1000);

  return blemac;
}

extern char* pendingAlarmUID;

void loop_onex()
{
  while(true){
    if(pendingAlarmUID){
      onex_run_evaluators(pendingAlarmUID, 0);
      free(pendingAlarmUID);
      pendingAlarmUID=0;
    }
    if(ticked){
      ticked=false;
      onex_run_evaluators(clockUID, 0);
    }
    if(!onex_loop()){
      time_delay_ms(5);
    }
  }
}

void set_blemac(char* blemac)
{
  object_property_set(config, (char*)"blemac", blemac);
}

