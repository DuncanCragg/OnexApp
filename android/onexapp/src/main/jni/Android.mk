
LOCAL_PATH := $(call my-dir)/../../../..

######################################################
include $(CLEAR_VARS)

LOCAL_MODULE := onexapp

LOCAL_C_INCLUDES :=
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/glm
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/imgui

PROJECT_FILES := $(wildcard $(LOCAL_PATH)/../sascha/*.cpp) $(wildcard $(LOCAL_PATH)/../src/*.cpp) $(wildcard $(LOCAL_PATH)/../external/imgui/*.cpp)

LOCAL_SRC_FILES := $(PROJECT_FILES)

LOCAL_CPPFLAGS := -std=c++11
LOCAL_CPPFLAGS += -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -DVK_NO_PROTOTYPES
LOCAL_CPPFLAGS += -DVK_USE_PLATFORM_ANDROID_KHR

LOCAL_LDLIBS := -landroid -llog -lz

LOCAL_STATIC_LIBRARIES += android_native_app_glue
LOCAL_STATIC_LIBRARIES += cpufeatures

LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/cpufeatures)
